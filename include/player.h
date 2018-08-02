#ifndef _PLAY_H_
#define _PLAY_H_
#include "queue.h"
#include "w32thread.h"
#include "bitstat.h"

enum 
{
    PS_NONE  = 0,
    PS_PLAY  = 1,
    PS_PAUSE = 2,
    PS_EXIT  = 3,
};

class VideoPlayer{
public:
    char*              m_pFilename;
    AVFormatContext*   m_pAvftx;
    AVCodecContext*    m_pAvctx;
    char*              m_strVideoInfo;
    PacketQueue        m_videoq;  // video packet queue
    FrameQueue         m_pictq;   // video decoded frame queue 
    struct SwsContext* m_pSwsCtx;
    int                m_iFrameInterval; // time interval between two frames
    AVRational         m_TimeBase;
    int                m_iWidth;
    int                m_iHeight;
    int                m_iRenderPicWidth;
    int                m_iRenderPicHeight;
    long long          m_llPlayStartTime;  // ms
    long long          m_llPlayPauseTime;
    int                m_iEof;

    BitStat*           m_CBitRateStat;   // stat bitrate per second
    BitStat*           m_CFrameBitsStat; // stat frame bit

    int                m_iPlayStat;
    pthread_mutex_t    m_mtxPlayStat;
    pthread_cond_t     m_condPlayCond;

    pthread_t          m_pReadThread;
    pthread_t          m_pDecThread;
    int                m_iReadThStat;
    int                m_iDecThStat;

    VideoPlayer(void);
    VideoPlayer(char* filename);
    ~VideoPlayer(void);

    void PlayerInit();
    void PlayerStart();
    void PlayerPause();
    void WaitUntilPaused();
    void PlayerExit();
    int  WaitToPlayOrExit();

    void InitScaleParameters(int dstWidth, int dstHeight);

    int  GetDuration();
    int  GetFileBitRate();
    int  GetStat();
    char* GetVideoInfo();

    static void* readThread(void* v);
    static void* decodeThread(void* v);
    //static void* renderThread(void* v);
};

#endif