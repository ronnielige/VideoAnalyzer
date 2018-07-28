//Implement gui control functions
#include "StdAfx.h"
#include "Form1.h"
#include "player.h"
#include "log.h"
using namespace VideoAnalyzer;

void Form1::PlayerInit()
{
    m_pl = (VideoPlayer*)malloc(sizeof(VideoPlayer));
    m_pl->frameInterval = 40; // default assume video fps = 25, then frame interval = 40 ms
    m_pl->width = 1280;
    m_pl->height = 720;
    m_pl->sws_ctx = NULL;
    m_pl->playPauseTime = m_pl->playPauseTime = 0;
    m_pl->avftx = avformat_alloc_context(); // init avformat context
    m_pl->eof = false;
    
    packet_queue_init(&(m_pl->videoq));
    picture_queue_init(&(m_pl->pictq));

    m_mtxPlayStat  = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    m_condPlayCond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    m_mtxRender    = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

    PlayStat = PS_NONE;    // init Player Stat 
    pthread_mutex_init(m_mtxPlayStat, NULL);
    pthread_cond_init(m_condPlayCond, NULL);
    pthread_mutex_init(m_mtxRender, NULL);

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

    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_EXIT;
    pthread_cond_broadcast(m_condPlayCond);  // send exit to threads
    pthread_mutex_unlock(m_mtxPlayStat);

    readThread->Join();   // wait read   thread to finish
    decThread->Join();    // wait decode thread to finish
    rendThread->Join();   // wait render thread to finish

    if(m_pl->avftx)
        avformat_close_input(&m_pl->avftx);

    if(m_pl->sws_ctx)
        sws_freeContext(m_pl->sws_ctx);

    packet_queue_destory(&(m_pl->videoq));
    picture_queue_destory(&(m_pl->pictq));
    free(m_pl);

    pthread_mutex_destroy(m_mtxPlayStat);
    pthread_cond_destroy(m_condPlayCond);
    pthread_mutex_destroy(m_mtxRender);
    free(m_mtxPlayStat);
    free(m_condPlayCond);
    free(m_mtxRender);
}

Form1::Form1(void)
{
    InitializeComponent();
    if(init_log() < 0) // can't open output log file
        exit(1);


    m_renderTlx = m_renderTly = 0;
    m_doscale = 0;
    m_renderAreaWidth  = VideoPlaybackPannel->Width;
    m_renderAreaHeight = VideoPlaybackPannel->Height;

    m_videoPlayGraphic = VideoPlaybackPannel->CreateGraphics();
    PlayerInit();
    mSetVidInfDelegate = gcnew setVideoInfo(this, &Form1::setVideoInfoMethod);
}

Form1::~Form1()
{
    PlayerExit();
    delete m_videoPlayGraphic;

    uninit_log();
    if (components)
    {
        delete components;
    }
}

System::Void Form1::VideoPlaybackPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    pthread_mutex_lock(m_mtxRender);

    delete m_videoPlayGraphic;
    m_videoPlayGraphic = VideoPlaybackPannel->CreateGraphics();
    m_videoPlayGraphic->Clear(Color::White);
    setRenderArea();
    if(m_pl->sws_ctx)
        sws_freeContext(m_pl->sws_ctx);
    if(m_doscale)
    {
        m_pl->sws_ctx = sws_getContext(m_pl->width, m_pl->height, AV_PIX_FMT_YUV420P,
                                       m_renderAreaWidth, m_renderAreaHeight, AV_PIX_FMT_BGR24, 
                                       SWS_BICUBIC, NULL, NULL, NULL);
        m_rpic = gcnew Bitmap(m_renderAreaWidth, m_renderAreaHeight, PixelFormat::Format24bppRgb); // TODO: cause memory leak?
        picture_queue_alloc_rgbframe(&(m_pl->pictq), m_renderAreaWidth, m_renderAreaHeight);
    }
    else
    {
        m_pl->sws_ctx = sws_getContext(m_pl->width, m_pl->height, AV_PIX_FMT_YUV420P,
                                       m_pl->width, m_pl->height, AV_PIX_FMT_BGR24, 
                                       SWS_BICUBIC, NULL, NULL, NULL);
        m_rpic = gcnew Bitmap(m_pl->width, m_pl->height, PixelFormat::Format24bppRgb); // TODO: cause memory leak?
        //picture_queue_alloc_rgbframe(&(m_pl->pictq), m_pl->width, m_pl->height);
    }

    pthread_mutex_unlock(m_mtxRender);
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
    va_log(LOGLEVEL_INFO, "setRenderArea: VideoRenderWindiw Size (%4d, %4d); RenderArea: topleft = (%4d, %4d), Size = (%4d, %4d)\n", pannelWidth, pannelHeight, m_renderTlx, m_renderTly, m_renderAreaWidth, m_renderAreaHeight);
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

    PlayerExit();
    PlayerInit();

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
    //m_videoPlayGraphic->Clear(Color::White);
}

System::Void Form1::RenderFrame(void) // render thread calls
{
    pthread_mutex_lock(m_mtxRender);
    Frame*     renderFrame  = picture_queue_get_read_picture(&(m_pl->pictq));
    int       rgbFrmWidth   = renderFrame->rgbframe->width;
    int      rgbFrmHeight   = renderFrame->rgbframe->height;

    if(m_doscale)
        sws_scale(m_pl->sws_ctx, 
                  renderFrame->yuvframe->data, renderFrame->yuvframe->linesize, 0, renderFrame->yuvframe->height, 
                  renderFrame->rgbframe->data, renderFrame->rgbframe->linesize);

    Drawing::Rectangle rect = Drawing::Rectangle(0, 0, rgbFrmWidth, rgbFrmHeight);

    BitmapData^ bmpData = m_rpic->LockBits(rect, ImageLockMode::ReadWrite, m_rpic->PixelFormat);
    IntPtr   bmpDataPtr = bmpData->Scan0;
    int           bytes = Math::Abs(bmpData->Stride) * m_rpic->Height;

    char* p = (char *)bmpDataPtr.ToPointer();
    memcpy(p, renderFrame->rgbframe->data[0], bytes * sizeof(char));

    m_rpic->UnlockBits(bmpData);
    
    va_log(LOGLEVEL_INFO, "Render Frame(pts = %6dms) Started, frameSize = (%4d, %4d), RenderArea: topleft = (%4d, %4d), RenderSize = (%4d, %4d)\n", (int)(1000 * renderFrame->pts), rgbFrmWidth, rgbFrmHeight, m_renderTlx, m_renderTly, m_renderAreaWidth, m_renderAreaHeight);
    m_videoPlayGraphic->DrawImage(m_rpic, m_renderTlx, m_renderTly, m_renderAreaWidth, m_renderAreaHeight);
    picture_queue_finish_read(&(m_pl->pictq));
    va_log(LOGLEVEL_INFO, "Render Frame(pts = %6dms) Ended\n", (int)(1000 * renderFrame->pts));
    pthread_mutex_unlock(m_mtxRender);
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