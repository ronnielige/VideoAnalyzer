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

System::Void readThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
    char* fname = NULL;
    int err;
    AVFormatContext* ic = avformat_alloc_context();
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

        if(mainForm->PlayStat == PS_INIT && !fname) // init
        {
            char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {"v"};
            int st_index[AVMEDIA_TYPE_NB] = {-1, -1, -1, -1};
            int video_st_index;
            AVCodecParameters *vcodecpar;
            String^ VidInfoStr;
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
            VidInfoStr += L"\nFile Format:\n  " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)ic->iformat->name) + "\n";

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

                VidInfoStr += L"\nResolution:\n  " + vcodecpar->width + " x " + vcodecpar->height + "\n"; 
				stream_component_open(ic, &avctx, &codec, st_index[AVMEDIA_TYPE_VIDEO]);
                VidInfoStr += L"\nVideo Codec:\n  " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)codec->name) + "\n"; 
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