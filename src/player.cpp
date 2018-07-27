#include "StdAfx.h"
#include "player.h"
#include "log.h"

extern "C"
{
#define __STDC_CONSTANT_MACROS
#include "libavutil\avutil.h"
#include "libavformat\avformat.h"
#include "libavcodec\avcodec.h"
};
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

int stream_component_open(AVFormatContext *ic, AVCodecContext** avctx, AVCodec** codec, int stream_index)
{
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    *avctx = ic->streams[stream_index]->codec;
    *codec = avcodec_find_decoder((*avctx)->codec_id);
    (*avctx)->codec_id = (*codec)->id;

    if(avcodec_open2(*avctx, *codec, NULL) < 0)
        return -1; 

    switch((*avctx)->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        //mForm->video_stream_index = stream_index;
        break;
    }
    return 0;
}

void dump_format(AVFormatContext* ic, int vid_stream_index, String^& videoInfo)
{
    char fmt_str[50];
    // Show Duration in Video Info List
    if(ic->duration != AV_NOPTS_VALUE)
    {   
        int hours, mins, secs, us, start_secs, start_us;
        int64_t duration = ic->duration + (ic->duration <= INT64_MAX - 5000 ? 5000 : 0);
        secs  = duration / AV_TIME_BASE;
        us    = duration % AV_TIME_BASE;
        mins  = secs / 60;
        secs %= 60;
        hours = mins / 60;
        mins %= 60;

        if (ic->start_time != AV_NOPTS_VALUE) {
            start_secs = llabs(ic->start_time / AV_TIME_BASE);
            start_us   = (int) av_rescale(llabs(ic->start_time % AV_TIME_BASE), 1000000, AV_TIME_BASE);
        }
        sprintf(fmt_str, "%02d:%02d:%02d.%02d \\ %s%d.%06d", hours, mins, secs, (100 * us) / AV_TIME_BASE, ic->start_time >= 0 ? "" : "-", start_secs, start_us);
        videoInfo += L"\nDuration \\ start:\n   " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)fmt_str) + "\n";
    }
    // Show Total File Bitrate in Video Info List
    if(ic->bit_rate)
    {
        sprintf(fmt_str, "%d kb/s", (int64_t)ic->bit_rate / 1000);
        videoInfo += L"\nFile BitRate:\n   " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)fmt_str) + "\n";
    }

    // Show Frame Rate in Video Info List
    AVStream *st = ic->streams[vid_stream_index];
    if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
    { 
        int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
        if(fps)
        {
            sprintf(fmt_str, " %5.2f", av_q2d(st->avg_frame_rate));
            videoInfo += L"\nFrame Rate:\n   " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)fmt_str) + "\n";
        }
    }
}

System::Void readThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
    char* fname = NULL;
    int err, ret;
    AVFormatContext* ic = NULL;
    AVCodecContext *avctx;// = mainForm->m_avctx;
    AVCodec* codec;// = mainForm->m_avcodec;
    AVPacket pkt1, *pkt = &pkt1;
    int64_t stream_start_time;
    int64_t pkt_ts;

    av_register_all();
    while(1)
    {
        pthread_mutex_lock(mainForm->m_mtxPlayStat);
        while(mainForm->PlayStat == PS_NONE || mainForm->PlayStat == PS_PAUSE) // wait until start play
            pthread_cond_wait(mainForm->m_condPlayCond, mainForm->m_mtxPlayStat);
        if(mainForm->PlayStat == PS_EXIT) // exit
        {
            pthread_mutex_unlock(mainForm->m_mtxPlayStat);
            break;
        }
        pthread_mutex_unlock(mainForm->m_mtxPlayStat);

        if(mainForm->PlayStat == PS_INIT) // init
        {
            char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
            int st_index[AVMEDIA_TYPE_NB] = {-1, -1, -1, -1};
            AVCodecParameters *vcodecpar;
            String^ VidInfoStr;
            if(ic)
                avformat_close_input(&ic);

            ic = avformat_alloc_context();
            if(!ic)
                return;
            fname = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(mainForm->mfilename);
            err = avformat_open_input(&ic, fname, NULL, NULL);
            if(err < 0)
            {
                if(ic)
                    avformat_close_input(&ic);
                return; 
            }
            err = avformat_find_stream_info(ic, NULL);
            if(err < 0)
            {
                if(ic)
                    avformat_close_input(&ic);
                return; 
            }
            VidInfoStr = L"\nFile:\n   " + mainForm->mfilename + "\n";
            VidInfoStr += L"\nFile Format:\n   " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)ic->iformat->long_name) + "\n";
            st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);

            if(st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
                AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
                vcodecpar = st->codecpar;
                AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
                VidInfoStr += L"\nResolution:\n   " + vcodecpar->width + " x " + vcodecpar->height + "\n"; 
                stream_component_open(ic, &avctx, &codec, st_index[AVMEDIA_TYPE_VIDEO]);

                mainForm->m_pl->frameInterval = 1000.0 / av_q2d(st->avg_frame_rate);
                mainForm->m_pl->width  = vcodecpar->width;
                mainForm->m_pl->height = vcodecpar->height;
                mainForm->video_stream_index = st_index[AVMEDIA_TYPE_VIDEO];
                VidInfoStr += L"\nVideo Codec:\n   " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)codec->long_name) + "\n"; 

                dump_format(ic, st_index[AVMEDIA_TYPE_VIDEO], VidInfoStr);
                if(vcodecpar->bit_rate)
                    VidInfoStr += L"\nVideo Bitrate:\n   " + (int)(vcodecpar->bit_rate / 1000) + " kbps\n";
            }

            mainForm->Invoke(mainForm->mSetVidInfDelegate, VidInfoStr);

            mainForm->m_avcodec = codec;
            mainForm->m_avctx   = avctx;
            mainForm->setRenderArea();
            if(mainForm->m_doscale)
            {
                mainForm->m_rpic = gcnew Bitmap(mainForm->m_renderAreaWidth, mainForm->m_renderAreaHeight, PixelFormat::Format24bppRgb);
                mainForm->m_pl->sws_ctx = sws_getContext(mainForm->m_pl->width, mainForm->m_pl->height, AV_PIX_FMT_YUV420P,
                                                         mainForm->m_renderAreaWidth, mainForm->m_renderAreaHeight, AV_PIX_FMT_BGR24, 
                                                         SWS_BICUBIC, NULL, NULL, NULL);
                picture_queue_alloc_rgbframe(&(mainForm->m_pl->pictq), mainForm->m_renderAreaWidth, mainForm->m_renderAreaHeight);
            }
            else
            {
                mainForm->m_rpic = gcnew Bitmap(mainForm->m_pl->width, mainForm->m_pl->height, PixelFormat::Format24bppRgb);
                mainForm->m_pl->sws_ctx = sws_getContext(mainForm->m_pl->width, mainForm->m_pl->height, AV_PIX_FMT_YUV420P,
                                                         mainForm->m_pl->width, mainForm->m_pl->height, AV_PIX_FMT_BGR24, 
                                                         SWS_BICUBIC, NULL, NULL, NULL);
                picture_queue_alloc_rgbframe(&(mainForm->m_pl->pictq), mainForm->m_pl->width, mainForm->m_pl->height);
            }


            pthread_mutex_lock(mainForm->m_mtxPlayStat);
            mainForm->PlayStat = PS_NONE;  // Finished Init, then just to wait
            pthread_cond_broadcast(mainForm->m_condPlayCond); 
            pthread_mutex_unlock(mainForm->m_mtxPlayStat);
        } // PS_INIT

        if(mainForm->PlayStat == PS_PLAY)
        {
            av_init_packet(pkt);
            pkt->data = NULL;
            pkt->size = 0;
            ret = av_read_frame(ic, pkt);
            if(ret < 0)
            {
                if((ret == AVERROR_EOF || avio_feof(ic->pb))) // reach end of file
                {
                }
            }
            stream_start_time = ic->streams[pkt->stream_index]->start_time;
            pkt_ts = pkt->pts == AV_NOPTS_VALUE? pkt->dts: pkt->pts;
            if(pkt->stream_index == AVMEDIA_TYPE_VIDEO)
            {
                packet_queue_put(&(mainForm->m_pl->videoq), pkt);
            }
        }
    }
    if(ic)
        avformat_close_input(&ic);
}

System::Void decodeThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
    AVPacket pkt1, *pkt = &pkt1;
    Frame* myframe;
    int got_frame, ret = 0;
    PacketQueue* pq = &(mainForm->m_pl->videoq);
    FrameQueue*  fq = &(mainForm->m_pl->pictq);
    while(1)
    {
        pthread_mutex_lock(mainForm->m_mtxPlayStat);
        if(mainForm->PlayStat == PS_EXIT) // exit
        {
            pthread_mutex_unlock(mainForm->m_mtxPlayStat);
            return;
        }
        while(mainForm->PlayStat != PS_PLAY && mainForm->PlayStat != PS_EXIT) // wait until start play
            pthread_cond_wait(mainForm->m_condPlayCond, mainForm->m_mtxPlayStat);
        pthread_mutex_unlock(mainForm->m_mtxPlayStat);

        if(mainForm->PlayStat == PS_PLAY)
        {
            packet_queue_get(pq, pkt);
            myframe = picture_queue_get_write_picture(fq);
            if(myframe)
            {
                do{
                    got_frame = 0;
                    ret = avcodec_decode_video2(mainForm->m_avctx, myframe->yuvframe, &got_frame, pkt);
                    if(ret < 0)
                        break;
                    pkt->data += pkt->size;
                    pkt->size -= pkt->size;
                }while(pkt->size > 0);

                if(got_frame)
                {
                    if(!mainForm->m_doscale)
                    {
                        sws_scale(mainForm->m_pl->sws_ctx, 
                                  myframe->yuvframe->data, myframe->yuvframe->linesize, 0, myframe->yuvframe->height, 
                                  myframe->rgbframe->data, myframe->rgbframe->linesize);
                    }
                    picture_queue_write(fq);
                }
            }
            av_packet_unref(pkt);

        }
    }
}

System::Void renderThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
    FrameQueue*  fq = &(mainForm->m_pl->pictq);
    Frame* pfrm;
    while(1)
    {
        pthread_mutex_lock(mainForm->m_mtxPlayStat);
        if(mainForm->PlayStat == PS_EXIT) // exit
        {
            pthread_mutex_unlock(mainForm->m_mtxPlayStat);
            return;
        }
        while(mainForm->PlayStat != PS_PLAY && mainForm->PlayStat != PS_EXIT) // wait until start play
            pthread_cond_wait(mainForm->m_condPlayCond, mainForm->m_mtxPlayStat);
        pthread_mutex_unlock(mainForm->m_mtxPlayStat);

        if(mainForm->PlayStat == PS_PLAY)
        {
            //va_log(LOGLEVEL_INFO, "RenderFrame Started\n");
            mainForm->RenderFrame();
            Sleep(10);
            //va_log(LOGLEVEL_INFO, "RenderFrame Ended\n");
            //Sleep(mainForm->m_pl->frameInterval);
        }
    }
}