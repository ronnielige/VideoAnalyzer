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
    if (stream_index < 0 || (unsigned int)stream_index >= ic->nb_streams)
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

void dump_format(AVFormatContext* ic, int vid_stream_index, char* videoInfo)
{
    // Show Duration in Video Info List
    if(ic->duration != AV_NOPTS_VALUE)
    {   
        int hours, mins, secs, us, start_secs, start_us;
        int64_t duration = ic->duration + (ic->duration <= INT64_MAX - 5000 ? 5000 : 0);
        secs  = (int)(duration / AV_TIME_BASE);
        us    = (int)(duration % AV_TIME_BASE);
        mins  = secs / 60;
        secs %= 60;
        hours = mins / 60;
        mins %= 60;

        if (ic->start_time != AV_NOPTS_VALUE) {
            start_secs = (int)llabs(ic->start_time / AV_TIME_BASE);
            start_us   = (int) av_rescale(llabs(ic->start_time % AV_TIME_BASE), 1000000, AV_TIME_BASE);
        }
        sprintf_s(videoInfo + strlen(videoInfo), 1000 - strlen(videoInfo), "\nDuration \\ start:\n   %02d:%02d:%02d.%02d \\ %s%d.%06d\n", hours, mins, secs, (100 * us) / AV_TIME_BASE, ic->start_time >= 0 ? "" : "-", start_secs, start_us);
    }
    // Show Total File Bitrate in Video Info List
    if(ic->bit_rate)
        sprintf_s(videoInfo + strlen(videoInfo), 1000 - strlen(videoInfo), "\nFile BitRate:\n   %d kb/s\n", (int64_t)ic->bit_rate / 1000);

    // Show Frame Rate in Video Info List
    AVStream *st = ic->streams[vid_stream_index];
    if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
    { 
        int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
        if(fps)
            sprintf_s(videoInfo + strlen(videoInfo), 1000 - strlen(videoInfo), "\nFrame Rate:\n    %5.2f\n", av_q2d(st->avg_frame_rate));
    }
}

VideoPlayer::VideoPlayer(void)
{
    m_pFilename = NULL;
    m_strVideoInfo = (char *)malloc(1000 * sizeof(char));
    m_iFrameInterval = 40; // assume video fps = 25, then frame interval = 40 ms
    m_iWidth  = 1280;
    m_iHeight = 720;
    m_pSwsCtx = NULL;
    m_llPlayStartTime = m_llPlayPauseTime = 0;
    m_pAvftx = avformat_alloc_context(); // init avformat context
    m_iEof = false;

    m_CBitRateStat   = new BitStat(1000);
    m_CFrameBitsStat = new BitStat(25000);

    packet_queue_init(&(m_videoq));
    picture_queue_init(&(m_pictq));

    m_iPlayStat = PS_NONE;    // init Player Stat 
    pthread_mutex_init(&m_mtxPlayStat, NULL);
    pthread_cond_init(&m_condPlayCond, NULL);
}

VideoPlayer::VideoPlayer(char* filename)
{
    m_pFilename    = (char*)malloc(strlen(filename) + 10);
    m_strVideoInfo = (char *)malloc(1000 * sizeof(char));
    strcpy(m_pFilename, filename);
    m_iFrameInterval = 40; // assume video fps = 25, then frame interval = 40 ms
    m_iWidth  = 1280;
    m_iHeight = 720;
    m_pSwsCtx = NULL;
    m_llPlayStartTime = m_llPlayPauseTime = 0;
    m_pAvftx = avformat_alloc_context(); // init avformat context
    m_iEof = false;

    m_CBitRateStat   = new BitStat(1000);
    m_CFrameBitsStat = new BitStat(25000);

    packet_queue_init(&(m_videoq));
    picture_queue_init(&(m_pictq));

    m_iPlayStat    = PS_NONE;    // init Player Stat 
    pthread_mutex_init(&m_mtxPlayStat, NULL);
    pthread_cond_init(&m_condPlayCond, NULL);
}

VideoPlayer::~VideoPlayer()
{
    if(m_pAvftx)
        avformat_close_input(&m_pAvftx);

    if(m_pSwsCtx)
        sws_freeContext(m_pSwsCtx);

    packet_queue_destory(&(m_videoq));
    picture_queue_destory(&(m_pictq));

    if(m_CBitRateStat)
        delete m_CBitRateStat;
    if(m_CFrameBitsStat)
        delete m_CFrameBitsStat;
    if(m_pFilename)
        free(m_pFilename);
    if(m_strVideoInfo)
        free(m_strVideoInfo);

    pthread_mutex_destroy(&m_mtxPlayStat);
    pthread_cond_destroy(&m_condPlayCond);
}

void VideoPlayer::PlayerInit()
{
    if(m_pFilename)
    {
        char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
        int st_index[AVMEDIA_TYPE_NB] = {-1, -1, -1, -1};
        AVCodecParameters *vcodecpar;
        int      err;
        AVCodec* codec;

        err = avformat_open_input(&m_pAvftx, m_pFilename, NULL, NULL);
        if(err < 0)
        {
            if(m_pAvftx)
                avformat_close_input(&m_pAvftx);
            return; 
        }
        err = avformat_find_stream_info(m_pAvftx, NULL);
        if(err < 0)
        {
            if(m_pAvftx)
                avformat_close_input(&m_pAvftx);
            va_log(LOGLEVEL_ERROR, "avformat_find_stream_info error\n");
            return; 
        }
        sprintf_s(m_strVideoInfo, 1000, "\nFile:\n   %s\n", m_pFilename);
        sprintf_s(m_strVideoInfo + strlen(m_strVideoInfo), 1000 - strlen(m_strVideoInfo), "\nFile Format:\n   %s\n", m_pAvftx->iformat->long_name);
        st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(m_pAvftx, AVMEDIA_TYPE_VIDEO, st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);

        if(st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
            AVStream *st = m_pAvftx->streams[st_index[AVMEDIA_TYPE_VIDEO]];
            vcodecpar = st->codecpar;
            AVRational sar = av_guess_sample_aspect_ratio(m_pAvftx, st, NULL);
            sprintf_s(m_strVideoInfo + strlen(m_strVideoInfo), 1000 - strlen(m_strVideoInfo), "\nResolution:\n   %d x %d\n", vcodecpar->width, vcodecpar->height);
            stream_component_open(m_pAvftx, &m_pAvctx, &codec, st_index[AVMEDIA_TYPE_VIDEO]);

            m_iFrameInterval = (int)(1000.0 / av_q2d(st->avg_frame_rate));
            m_TimeBase = st->time_base;
            m_iWidth   = vcodecpar->width;
            m_iHeight  = vcodecpar->height;
            sprintf_s(m_strVideoInfo + strlen(m_strVideoInfo), 1000 - strlen(m_strVideoInfo), "\nVideo Codec:\n   %s\n", codec->long_name);
            dump_format(m_pAvftx, st_index[AVMEDIA_TYPE_VIDEO], m_strVideoInfo);
        }

        pthread_create(&m_pReadThread, NULL, readThread, (void*)this);
        pthread_create(&m_pDecThread,  NULL, decodeThread, (void*)this);
    }
}

void VideoPlayer::InitScaleParameters(int dstWidth, int dstHeight)
{
    if(m_pSwsCtx)
        sws_freeContext(m_pSwsCtx);

    m_pSwsCtx = sws_getContext(m_iWidth, m_iHeight, AV_PIX_FMT_YUV420P,
                               dstWidth, dstHeight, AV_PIX_FMT_BGR24, 
                               SWS_BICUBIC, NULL, NULL, NULL);
    picture_queue_alloc_rgbframe(&(m_pictq), dstWidth, dstHeight);
}

void VideoPlayer::PlayerStart()
{
    packet_queue_abort(&(m_videoq), false);
    picture_queue_abort(&(m_pictq), false);

    pthread_mutex_lock(&m_mtxPlayStat);
    m_iPlayStat = PS_PLAY;
    pthread_cond_broadcast(&m_condPlayCond);  // send play command to threads
    pthread_mutex_unlock(&m_mtxPlayStat);
    va_log(LOGLEVEL_INFO, "Start to Play\n");
}

void VideoPlayer::PlayerPause()
{
    pthread_mutex_lock(&m_mtxPlayStat);
    m_iPlayStat = PS_PAUSE;
    pthread_cond_broadcast(&m_condPlayCond);  // send play command to threads
    pthread_mutex_unlock(&m_mtxPlayStat);

    va_log(LOGLEVEL_INFO, "Stop Play\n");
}

void VideoPlayer::WaitUntilPaused()
{
    packet_queue_abort(&(m_videoq), true);
    picture_queue_abort(&(m_pictq), true);
    while(m_iReadThStat != PS_PAUSE || m_iDecThStat != PS_PAUSE)
        Sleep(1);
}

int VideoPlayer::WaitToPlayOrExit()
{
    pthread_mutex_lock(&m_mtxPlayStat);
    while(m_iPlayStat == PS_NONE || m_iPlayStat == PS_PAUSE) // wait until start play
        pthread_cond_wait(&m_condPlayCond, &m_mtxPlayStat);

    if(m_iPlayStat == PS_EXIT) // exit
    {
        pthread_mutex_unlock(&m_mtxPlayStat);
        return PS_EXIT;
    }
    pthread_mutex_unlock(&m_mtxPlayStat);
    return PS_PLAY;
}

void VideoPlayer::PlayerExit()
{
    packet_queue_abort(&(m_videoq), true); 
    picture_queue_abort(&(m_pictq), true);

    pthread_mutex_lock(&m_mtxPlayStat);
    m_iPlayStat = PS_EXIT;
    pthread_cond_broadcast(&m_condPlayCond);  // send exit to threads
    pthread_mutex_unlock(&m_mtxPlayStat);

    pthread_join(m_pReadThread, NULL);   // wait read   thread to finish
    pthread_join(m_pDecThread, NULL);    // wait decode thread to finish
}

int VideoPlayer::GetDuration()
{
    int duration_sec = (int)((m_pAvftx->duration - m_pAvftx->start_time) / AV_TIME_BASE);
    return duration_sec;
}

int VideoPlayer::GetFileBitRate() // bps
{
    return (int)(m_pAvftx->bit_rate);
}

int VideoPlayer::GetStat()
{
    return m_iPlayStat;
}

char* VideoPlayer::GetVideoInfo()
{
    return m_strVideoInfo;
}

void* VideoPlayer::readThread(void* v)
{
    VideoPlayer* pl = (VideoPlayer*)v;
    char* fname = NULL;
    int ret;
    AVFormatContext* fmtctx = pl->m_pAvftx;
    AVPacket pkt1, *pkt = &pkt1;
    AVPacket flush_pkt1, *flush_pkt = &flush_pkt1;
    int64_t stream_start_time;
    int64_t pkt_ts;

    av_register_all();
    while(1)
    {
        pthread_mutex_lock(&pl->m_mtxPlayStat);
        while(pl->m_iPlayStat == PS_NONE || pl->m_iPlayStat == PS_PAUSE) // wait until start play
        {
            pl->m_iReadThStat = PS_PAUSE;
            pthread_cond_wait(&pl->m_condPlayCond, &pl->m_mtxPlayStat);
        }
        if(pl->m_iPlayStat == PS_EXIT) // exit
        {
            pthread_mutex_unlock(&pl->m_mtxPlayStat);
            break; 
        }
        pthread_mutex_unlock(&pl->m_mtxPlayStat);

        if(pl->m_iPlayStat == PS_PLAY)
        {
            pl->m_iReadThStat = PS_PLAY;
            av_init_packet(pkt);
            pkt->data = NULL;
            pkt->size = 0;
            ret = av_read_frame(fmtctx, pkt);
            if(ret < 0)
            {
                if((ret == AVERROR_EOF || avio_feof(fmtctx->pb))) // reach end of file
                {
                    pl->m_iEof = true;
                    flush_pkt->data = NULL;
                    flush_pkt->size = 0;
                    packet_queue_put(&(pl->m_videoq), flush_pkt);
                    va_log(LOGLEVEL_INFO, "readThread reached end of file, put null packet\n");
                    break;
                }
                if(fmtctx->pb && fmtctx->pb->error)
                    continue;
                else
                    continue;
            }
            stream_start_time = fmtctx->streams[pkt->stream_index]->start_time;
            pkt_ts = pkt->pts == AV_NOPTS_VALUE? pkt->dts: pkt->pts;
            if(pkt->stream_index == AVMEDIA_TYPE_VIDEO)
                packet_queue_put(&(pl->m_videoq), pkt);
        }
    }
    return NULL;
}

void* VideoPlayer::decodeThread(void* v)
{
    VideoPlayer* pl = (VideoPlayer*)v;
    AVPacket pkt1, *pkt = &pkt1;
    Frame* myframe;
    int got_frame, ret = 0;
    PacketQueue* pq = &(pl->m_videoq);
    FrameQueue*  fq = &(pl->m_pictq);
    int frame_pkt_size = 0;
    while(1)
    {
        pthread_mutex_lock(&pl->m_mtxPlayStat);
        if(pl->m_iPlayStat == PS_EXIT) // exit
        {
            pthread_mutex_unlock(&pl->m_mtxPlayStat);
            break;
        }
        while(pl->m_iPlayStat != PS_PLAY && pl->m_iPlayStat != PS_EXIT) // wait until start play
        {
            pl->m_iDecThStat = PS_PAUSE;
            pthread_cond_wait(&pl->m_condPlayCond, &pl->m_mtxPlayStat);
        }
        pthread_mutex_unlock(&pl->m_mtxPlayStat);

        if(pl->m_iPlayStat == PS_PLAY)
        {
            pl->m_iDecThStat = PS_PLAY;
            ret = packet_queue_get(pq, pkt);
            myframe = picture_queue_get_write_picture(fq);
            if(myframe && ret >= 0)
            {
                frame_pkt_size += pkt->size;
                do{
                    got_frame = 0;
                    ret = avcodec_decode_video2(pl->m_pAvctx, myframe->yuvframe, &got_frame, pkt);
                    if(ret < 0)
                        break;
                    pkt->data += pkt->size;
                    pkt->size -= pkt->size;
                }while(pkt->size > 0);
                
                if(got_frame)
                {
                    myframe->yuvframe->pts = av_frame_get_best_effort_timestamp(myframe->yuvframe);
                    myframe->pts = (myframe->yuvframe->pts == AV_NOPTS_VALUE) ? NAN : myframe->yuvframe->pts * av_q2d(pl->m_TimeBase);
                    myframe->frame_pkt_bits = frame_pkt_size * 8;
                    frame_pkt_size = 0;
                    if(1)
                    {
                        va_log(LOGLEVEL_INFO, "Decode frame pts = %8d ms, sws_scale width, height = %d, %d\n", (int)(1000 * myframe->pts), myframe->rgbframe->width, myframe->rgbframe->height);
                        sws_scale(pl->m_pSwsCtx, 
                                  myframe->yuvframe->data, myframe->yuvframe->linesize, 0, myframe->yuvframe->height, 
                                  myframe->rgbframe->data, myframe->rgbframe->linesize);
                        myframe->b_rgbready = true;
                    }
                    va_log(LOGLEVEL_FULL, "Decoded frame pts = %8d ms, %lld \n", (int)(1000 * myframe->pts), DateTime::Now.ToFileTime() / 10000);
                    picture_queue_write(fq);
                }

                if(pkt->data == NULL) // get flush pkt
                {
                    do{
                        ret = avcodec_decode_video2(pl->m_pAvctx, myframe->yuvframe, &got_frame, pkt);
                        if(ret < 0)
                            break;
                        if(got_frame)
                        {
                            // TODO: need to append these frames
                        }
                    }while(got_frame);
                    va_log(LOGLEVEL_INFO, "decodeThread received null packet, end of file and exit thread\n");
                    break; // exit decode thread
                }
            }
            av_packet_unref(pkt); // release pkt buffer
        }
    }
    return NULL;
}