#include "StdAfx.h"
#include "player.h"

System::Void readThreadProc(Object^ data)
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
        mainForm->set_resolution(L"Hello World");
        while(mainForm->PlayStat != PS_PLAY && mainForm->PlayStat != PS_EXIT) // wait until start play
            pthread_cond_wait(mainForm->m_condPlayCond, mainForm->m_mtxPlayStat);
        pthread_mutex_unlock(mainForm->m_mtxPlayStat);
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
        mainForm->set_resolution(L"Hello World");
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
        mainForm->set_resolution(L"Hello World");
        while(mainForm->PlayStat != PS_PLAY && mainForm->PlayStat != PS_EXIT) // wait until start play
            pthread_cond_wait(mainForm->m_condPlayCond, mainForm->m_mtxPlayStat);
        pthread_mutex_unlock(mainForm->m_mtxPlayStat);
    }
}