//Implement gui control functions
#include "StdAfx.h"
#include "Form1.h"
#include "player.h"
#include "log.h"
using namespace VideoAnalyzer;

void packet_queue_init(PacketQueue* pq)
{
    pq->size    = 0;
    pq->maxsize = 30;
    pq->abort   = 0;
    pq->firstNode = pq->lastNode = NULL;
    pq->mtx  = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pq->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(pq->mtx, NULL);
    pthread_cond_init(pq->cond, NULL);
}

void packet_queue_destory(PacketQueue* pq)
{
    PacketListNode* nd = pq->firstNode;
    while(nd)
    {
        pq->firstNode = pq->firstNode->next; // move to next
        free(nd);
        nd = pq->firstNode;
    }
    pthread_mutex_destroy(pq->mtx);
    pthread_cond_destroy(pq->cond);
    free(pq->mtx);
    free(pq->cond);
}

void packet_queue_put(PacketQueue* pq, AVPacket* pkt)
{
    pthread_mutex_lock(pq->mtx);
    if(pq->firstNode == NULL) // queue empty
    {
        pq->firstNode = (PacketListNode *)malloc(sizeof(PacketListNode));
        pq->firstNode->pkt  = *pkt;
        pq->firstNode->next = NULL;
        pq->lastNode = pq->firstNode;
        pq->size++;
    }
    else
    {
        while(pq->size >= pq->maxsize && !pq->abort) // packet queue full
            pthread_cond_wait(pq->cond, pq->mtx);

        PacketListNode* nnode = (PacketListNode *)malloc(sizeof(PacketListNode));
        nnode->pkt  = *pkt;
        nnode->next = NULL;
        pq->lastNode->next = nnode;
        pq->lastNode = pq->lastNode->next;
        pq->size++;
    }
    pthread_cond_signal(pq->cond);
    pthread_mutex_unlock(pq->mtx);
}

void packet_queue_get(PacketQueue* pq, AVPacket *rpkt)
{
    PacketListNode* hdNode;
    pthread_mutex_lock(pq->mtx);
    while(!pq->abort && (pq->size == 0 || pq->firstNode == NULL)) // packet queue empty, wait
        pthread_cond_wait(pq->cond, pq->mtx);
    hdNode = pq->firstNode;
    *rpkt = pq->firstNode->pkt;
    pq->firstNode = pq->firstNode->next;
    free(hdNode);
    pq->size--;
    pthread_cond_signal(pq->cond); // inform 
    pthread_mutex_unlock(pq->mtx);
}

void packet_queue_abort(PacketQueue* pq)
{
    pthread_mutex_lock(pq->mtx);
    pq->abort = 1;
    pthread_cond_signal(pq->cond);
    pthread_mutex_unlock(pq->mtx);
}

int picture_queue_init(FrameQueue* fq)
{
    fq->size = fq->ridx = fq->widx = 0;
    fq->abort = 0;
    fq->max_size = FRAME_QUEUE_SIZE;
    fq->mtx  = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    fq->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    for(int i = 0; i < fq->max_size; i++)
    {
        fq->fqueue[i].yuvframe = av_frame_alloc();
        fq->fqueue[i].rgbframe = NULL;
        if(fq->fqueue[i].yuvframe == NULL)
            return AVERROR(ENOMEM);
    }
    pthread_mutex_init(fq->mtx, NULL);
    pthread_cond_init(fq->cond, NULL);
    return 0;
}

int picture_queue_alloc_rgbframe(FrameQueue* fq, int width, int height)
{
    int ret;
    for(int i = 0; i < fq->max_size; i++)
    {
        av_frame_free(&(fq->fqueue[i].rgbframe)); // first free, then alloc

        fq->fqueue[i].rgbframe = av_frame_alloc();
        if(fq->fqueue[i].rgbframe == NULL)
            return AVERROR(ENOMEM);
        fq->fqueue[i].rgbframe->format = AV_PIX_FMT_BGR24;
        fq->fqueue[i].rgbframe->width  = width;
        fq->fqueue[i].rgbframe->height = height;

        ret = av_frame_get_buffer(fq->fqueue[i].rgbframe, 32);
        if(ret < 0)
            return -1;
    }
    return 0;
}

void picture_queue_destory(FrameQueue* fq)
{
    for(int i = 0; i < fq->max_size; i++)
    {
        Frame* vp = &fq->fqueue[i];
        //av_frame_unref(vp->yuvframe);
        //av_frame_unref(vp->rgbframe);
        av_frame_free(&vp->yuvframe);
        av_frame_free(&vp->rgbframe);
    }
    pthread_mutex_destroy(fq->mtx);
    pthread_cond_destroy(fq->cond);
    free(fq->mtx);
    free(fq->cond);
}

Frame* picture_queue_get_write_picture(FrameQueue* fq)
{
    pthread_mutex_lock(fq->mtx);
    while(fq->size >= FRAME_QUEUE_SIZE && !fq->abort)  // frame queue full
        pthread_cond_wait(fq->cond, fq->mtx);
    pthread_mutex_unlock(fq->mtx);
    if(fq->abort)
        return NULL;
    return &(fq->fqueue[fq->widx]);
}

void picture_queue_write(FrameQueue* fq)
{
    pthread_mutex_lock(fq->mtx);
    fq->widx = ((fq->widx + 1) == FRAME_QUEUE_SIZE)? 0: fq->widx + 1; // update write index
    fq->size++;
    pthread_cond_signal(fq->cond);
    pthread_mutex_unlock(fq->mtx);
}

Frame* picture_queue_read(FrameQueue* fq)
{
    Frame* rf;
    pthread_mutex_lock(fq->mtx);
    while(fq->size == 0 && !fq->abort)
        pthread_cond_wait(fq->cond, fq->mtx);  // frame queue empty
    rf = &(fq->fqueue[fq->ridx]);
    fq->size--;
    fq->ridx = ((fq->ridx + 1) == FRAME_QUEUE_SIZE)? 0: fq->ridx + 1; //
    pthread_cond_signal(fq->cond);
    pthread_mutex_unlock(fq->mtx);
    return rf;
}

void picture_queue_abort(FrameQueue* fq)
{
    pthread_mutex_lock(fq->mtx);
    fq->abort = 1;
    pthread_cond_signal(fq->cond);
    pthread_mutex_unlock(fq->mtx);
}

void Form1::PlayerInit()
{
    m_pl = (VideoPlayer*)malloc(sizeof(VideoPlayer));
    m_pl->frameInterval = 40; // default assume video fps = 25, then frame interval = 40 ms
    m_pl->width = 1280;
    m_pl->height = 720;
    m_pl->sws_ctx = NULL;
    packet_queue_init(&(m_pl->videoq));
    picture_queue_init(&(m_pl->pictq));
    readThread = gcnew Thread(gcnew ParameterizedThreadStart(&readThreadProc));
    readThread->Start(this);

    decThread = gcnew Thread(gcnew ParameterizedThreadStart(&decodeThreadProc));
    decThread->Start(this);

    rendThread = gcnew Thread(gcnew ParameterizedThreadStart(&renderThreadProc));
    rendThread->Start(this);
}

void Form1::PlayerExit()
{
    packet_queue_abort(&(m_pl->videoq));
    picture_queue_abort(&(m_pl->pictq));
    if(m_pl->sws_ctx)
        sws_freeContext(m_pl->sws_ctx);
}

Form1::Form1(void)
{
    InitializeComponent();
    PlayStat = PS_NONE;    // init Player Stat 
    m_mtxPlayStat  = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    m_condPlayCond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(m_mtxPlayStat, NULL);
    pthread_cond_init(m_condPlayCond, NULL);

    m_renderTlx = m_renderTly = 0;
    m_renderAreaWidth  = VideoPlaybackPannel->Width;
    m_renderAreaHeight = VideoPlaybackPannel->Height;

    videoPlayGraphic = VideoPlaybackPannel->CreateGraphics();
    PlayerInit();
    mSetVidInfDelegate = gcnew setVideoInfo(this, &Form1::setVideoInfoMethod);

    init_log();
}

Form1::~Form1()
{
    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_EXIT;
    pthread_cond_broadcast(m_condPlayCond);  // send exit to threads
    pthread_mutex_unlock(m_mtxPlayStat);

    PlayerExit();
    delete videoPlayGraphic;

    readThread->Join();   // wait read   thread to finish
    decThread->Join();    // wait decode thread to finish
    rendThread->Join();   // wait render thread to finish

    packet_queue_destory(&(m_pl->videoq));
    picture_queue_destory(&(m_pl->pictq));
    free(m_pl);
    //m_rpic->Dispose();

    pthread_mutex_destroy(m_mtxPlayStat);
    pthread_cond_destroy(m_condPlayCond);
    free(m_mtxPlayStat);
    free(m_condPlayCond);
    
    uninit_log();
    if (components)
    {
        delete components;
    }
}

System::Void Form1::VideoPlaybackPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    delete videoPlayGraphic;
    videoPlayGraphic = VideoPlaybackPannel->CreateGraphics();
    videoPlayGraphic->Clear(Color::White);
    setRenderArea();
    if(m_pl->sws_ctx)
        sws_freeContext(m_pl->sws_ctx);
    m_pl->sws_ctx = sws_getContext(m_pl->width, m_pl->height, AV_PIX_FMT_YUV420P,
                                   m_renderAreaWidth, m_renderAreaHeight, AV_PIX_FMT_BGR24, 
                                   SWS_BICUBIC, NULL, NULL, NULL);
    m_rpic = gcnew Bitmap(m_renderAreaWidth, m_renderAreaHeight, PixelFormat::Format24bppRgb); // TODO: cause memory leak?
    picture_queue_alloc_rgbframe(&(m_pl->pictq), m_renderAreaWidth, m_renderAreaHeight);
}

System::Void Form1::VideoBitratePannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    Graphics^ g = VideoBitratePannel->CreateGraphics();
    g->Clear(Color::White);
    drawGrid(g, VideoBitratePannel->Width, VideoBitratePannel->Height, 20, 20, 5);
    delete g;
}

System::Void Form1::VBVBufferPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    Graphics^ g = VBVBufferPannel->CreateGraphics();
    g->Clear(Color::White);
    drawGrid(g, VBVBufferPannel->Width, VBVBufferPannel->Height, 20, 20, 5);
    delete g;
}

System::Void Form1::setRenderArea()
{
    Int32     picWidth = m_pl->width;
    Int32    picHeight = m_pl->height;
    Int32  pannelWidth = VideoPlaybackPannel->Width;
    Int32 pannelHeight = VideoPlaybackPannel->Height;
    float      fScaleY = (float)pannelHeight / picHeight; 
    float      fScaleX = (float)pannelWidth  / picWidth; 
    // scale picture resolution to fit render area
    Int32 ScaledHeight = pannelHeight;      // first fill y
    Int32  ScaledWidth = picWidth * pannelHeight / picHeight; // first scale x
    if(ScaledWidth > pannelWidth)
    {
        ScaledWidth  = pannelWidth;
        ScaledHeight = picHeight * pannelWidth  / picWidth;
    }
    m_renderTlx = (pannelWidth - ScaledWidth) >> 1;
    m_renderTly = (pannelHeight - ScaledHeight) >> 1;
    m_renderAreaWidth  = ScaledWidth;
    m_renderAreaHeight = ScaledHeight;
}

System::Void Form1::drawGrid(Graphics^ g, Int32 Width, Int32 Height, Int32 GridSize, Int32 ipadx, Int32 ipady)
{
    Pen^ pen = gcnew Pen(Color::Gray, 1.0);
    Point^ tl = gcnew Point(); // top left position of drawing area
    tl->X = ipadx;
    tl->Y = ipady;
    Point^ br = gcnew Point(); // bottom right position of drawing area
    br->X = ipadx + (Width - ipadx) / GridSize * GridSize;
    br->Y = ipady + (Height - ipady) / GridSize * GridSize;
    Point^ bl = gcnew Point(); // bottom left position of drawing area
    bl->X = tl->X;
    bl->Y = br->Y;
    Point^ tr = gcnew Point(); // bottom left position of drawing area
    tr->X = br->X;
    tr->Y = tl->Y;

    pen->DashStyle = DashStyle::DashDot;
    // vertical grid line
    for(Int32 i = tl->X; i <= br->X; i += GridSize)
        g->DrawLine(pen, i, tl->Y, i, br->Y);
    // horizontal grid line
    for(Int32 i = tl->Y; i <= br->Y; i += GridSize)
        g->DrawLine(pen, tl->X, i, br->X, i);

    pen->Width = 2.0;
    pen->DashStyle = DashStyle::Solid;
    g->DrawLine(pen, tl->X, tl->Y, tr->X, tr->Y);
    g->DrawLine(pen, tl->X, tl->Y, bl->X, bl->Y);
    g->DrawLine(pen, bl->X, bl->Y, br->X, br->Y);
    g->DrawLine(pen, tr->X, tr->Y, br->X, br->Y);
}

System::Void Form1::openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
{
    // First reset Play stat
    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_NONE;
    pthread_cond_broadcast(m_condPlayCond);  // send init command to threads
    pthread_mutex_unlock(m_mtxPlayStat);

    openFileDialog1->ShowDialog();
    mfilename = openFileDialog1->FileName;
    this->Text = L"VideoAnalyzer " + mfilename;
    va_log(LOGLEVEL_INFO, "Open File %s\n", (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(mfilename));

    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_INIT;
    pthread_cond_broadcast(m_condPlayCond);  // send init command to threads
    pthread_mutex_unlock(m_mtxPlayStat);
}

System::Void Form1::StopButton_Click(System::Object^  sender, System::EventArgs^  e) 
{
    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_PAUSE;
    pthread_cond_broadcast(m_condPlayCond);  // send play command to threads
    pthread_mutex_unlock(m_mtxPlayStat);
    va_log(LOGLEVEL_INFO, "Stop Play\n");
    //videoPlayGraphic->Clear(Color::White);
}

System::Void Form1::RenderFrame(void)
{
    Frame*     renderFrame  = picture_queue_read(&(m_pl->pictq));
    int       rgbFrmWidth   = renderFrame->rgbframe->width;
    int      rgbFrmHeight   = renderFrame->rgbframe->height;
    Drawing::Rectangle rect = Drawing::Rectangle(0, 0, rgbFrmWidth, rgbFrmHeight);

    BitmapData^ bmpData = m_rpic->LockBits(rect, ImageLockMode::ReadWrite, m_rpic->PixelFormat);
    IntPtr   bmpDataPtr = bmpData->Scan0;
    int           bytes = Math::Abs(bmpData->Stride) * m_rpic->Height;

    char* p = (char *)bmpDataPtr.ToPointer();
    memcpy(p, renderFrame->rgbframe->data[0], bytes * sizeof(char));

    m_rpic->UnlockBits(bmpData);
    va_log(LOGLEVEL_INFO, "Show Frame Started\n");
    videoPlayGraphic->DrawImage(m_rpic, m_renderTlx, m_renderTly, m_renderAreaWidth, m_renderAreaHeight);
    va_log(LOGLEVEL_INFO, "Show Frame Ended\n");
}

System::Void Form1::PlayButton_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if(String::IsNullOrEmpty(mfilename)) // input file not choosen yet
        return;

    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_PLAY;
    pthread_cond_broadcast(m_condPlayCond);  // send play command to threads
    pthread_mutex_unlock(m_mtxPlayStat);

    va_log(LOGLEVEL_INFO, "Start to Play\n");
}