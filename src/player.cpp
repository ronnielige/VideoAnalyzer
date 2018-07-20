#include "StdAfx.h"
#include "player.h"

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
    int err;
    AVFormatContext* ic;
	AVCodecContext *avctx;
	AVCodec* codec;

    int i;
    if(!ic)
        return;

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
            char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {"v"};
            int st_index[AVMEDIA_TYPE_NB] = {-1, -1, -1, -1};
            int video_st_index;
            AVCodecParameters *vcodecpar;
            String^ VidInfoStr;
            ic = avformat_alloc_context();
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

            for( i = 0; i < ic->nb_streams; i++)
            {
                AVStream* st = ic->streams[i];
                enum AVMediaType type = st->codecpar->codec_type;
                st->discard = AVDISCARD_ALL;
                if (wanted_stream_spec[type] && st_index[type] == -1)
                    if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0)
                        st_index[type] = i;
            }
            for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
                if (wanted_stream_spec[i] && st_index[i] == -1) {
                    av_log(NULL, AV_LOG_ERROR, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string((enum AVMediaType)i));
                    st_index[i] = INT_MAX;
                }
            }
            st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
            if(st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
                AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
                vcodecpar = st->codecpar;
                AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
                VidInfoStr += L"\nResolution:\n   " + vcodecpar->width + " x " + vcodecpar->height + "\n"; 
				stream_component_open(ic, &avctx, &codec, st_index[AVMEDIA_TYPE_VIDEO]);
                VidInfoStr += L"\nVideo Codec:\n   " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)codec->long_name) + "\n"; 

                dump_format(ic, st_index[AVMEDIA_TYPE_VIDEO], VidInfoStr);
                if(vcodecpar->bit_rate)
                    VidInfoStr += L"\nVideo Bitrate:\n   " + (int)(vcodecpar->bit_rate / 1000) + " kbps\n";
            }

            mainForm->Invoke(mainForm->mSetVidInfDelegate, VidInfoStr);

			pthread_mutex_lock(mainForm->m_mtxPlayStat);
			mainForm->PlayStat = PS_NONE;  // Finished Init, then just to wait
			pthread_cond_broadcast(mainForm->m_condPlayCond); 
			pthread_mutex_unlock(mainForm->m_mtxPlayStat);
        }
    }
}

System::Void decodeThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
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
    }
}

System::Void renderThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
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
    }
}