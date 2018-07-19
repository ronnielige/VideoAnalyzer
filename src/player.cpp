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

System::Void readThreadProc(Object^ data)
{
    Form1^ mainForm = (Form1^)data;
    char* fname = NULL;
    int err;
    AVFormatContext* ic = avformat_alloc_context();
    if(!ic)
    {
        return;
    }
    while(1)
    {
        pthread_mutex_lock(mainForm->m_mtxPlayStat);
        //mainForm->set_resolution(L"Hello World");
        while(mainForm->PlayStat != PS_PLAY && mainForm->PlayStat != PS_EXIT) // wait until start play
            pthread_cond_wait(mainForm->m_condPlayCond, mainForm->m_mtxPlayStat);
        if(mainForm->PlayStat == PS_EXIT) // exit
        {
            pthread_mutex_unlock(mainForm->m_mtxPlayStat);
            break;
        }
        pthread_mutex_unlock(mainForm->m_mtxPlayStat);

        if(mainForm->PlayStat == PS_PLAY && !fname) // init
        {
            fname = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(mainForm->mfilename);
            err = avformat_open_input(&ic, fname, NULL, NULL);
            String^ format_name = System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)ic->iformat->name);

            mainForm->Invoke(mainForm->mSetResDelegate, format_name);
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