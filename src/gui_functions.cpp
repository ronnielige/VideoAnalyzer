//Implement gui control functions
#include "StdAfx.h"
#include "Form1.h"
#include "player.h"
#include "log.h"
using namespace VideoAnalyzer;

Form1::Form1(void)
{
    InitializeComponent();
    if(init_log() < 0) // can't open output log file
        exit(1);

    m_renderTlx = m_renderTly = 0;
    m_doscale = 0;  // TODO: =1 reduce ShowFrame time consuming, but brings crash problem. need to fix it.
    m_renderAreaWidth  = VideoPlaybackPannel->Width;
    m_renderAreaHeight = VideoPlaybackPannel->Height;

    m_videoPlayGraphic = VideoPlaybackPannel->CreateGraphics();
    m_CPlayer = NULL;
    m_CBitRateStat   = new BitStat(1000);
    m_CFrameBitsStat = new BitStat(25000);

    mSetVidInfDelegate = gcnew setVideoInfo(this, &Form1::setVideoInfoMethod);
    msetBitRatePicBoxWidthDelegate = gcnew setBitRatePicBoxWidthDelegate(this, &Form1::setBitRatePicBoxWidthMethod);
    msetBitRatePannelHScrollDelegate = gcnew setBitRatePannelHScrollDelegate(this, &Form1::setBRPannelHScrollMethod);

    this->SetStyle(static_cast<ControlStyles>(ControlStyles::DoubleBuffer | ControlStyles::UserPaint | ControlStyles::AllPaintingInWmPaint), true);
    this->UpdateStyles();

    m_oscBitRate = gcnew oscillogram(VideoBitRatePicBox, VideoBitRatePicBox->Width, VideoBitRatePicBox->Height);
}

Form1::~Form1()
{
    if(m_CBitRateStat)
        delete m_CBitRateStat;
    if(m_CFrameBitsStat)
        delete m_CFrameBitsStat;

    if(m_CPlayer)
    {
        m_CPlayer->PlayerExit();
        rendThread->Join();  // wait render thread exit first and then delete player
        delete m_CPlayer;
    }
 
    delete m_videoPlayGraphic;
    uninit_log();
    if (components)
    {
        delete components;
    }
}

System::Void Form1::VideoPlaybackPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    if(m_CPlayer)  // need to reallocate if player has been created
    {
        Int32 orgPlayStat = m_CPlayer->GetStat();
        if(orgPlayStat == PS_PLAY) // first pause read, decode and render thread
        {
            m_CPlayer->PlayerPause();
            m_CPlayer->WaitUntilPaused();
        }

        setRenderArea();

        if(m_doscale)
        {
            m_rpic = gcnew Bitmap(m_renderAreaWidth, m_renderAreaHeight, PixelFormat::Format24bppRgb);
            m_CPlayer->InitScaleParameters(m_renderAreaWidth, m_renderAreaHeight);
            va_log(LOGLEVEL_INFO, "VideoPlayBackPannel Resize, new sws_ctx width, height = %d, %d\n", m_renderAreaWidth, m_renderAreaHeight);
        }
        else  // nothing to do
        {
            //m_pl->sws_ctx = sws_getContext(m_pl->width, m_pl->height, AV_PIX_FMT_YUV420P,
            //                               m_pl->width, m_pl->height, AV_PIX_FMT_BGR24, 
            //                               SWS_BICUBIC, NULL, NULL, NULL);
            //m_rpic = gcnew Bitmap(m_pl->width, m_pl->height, PixelFormat::Format24bppRgb); // TODO: cause memory leak?
            //picture_queue_alloc_rgbframe(&(m_pl->pictq), m_pl->width, m_pl->height);
        }

        delete m_videoPlayGraphic;
        m_videoPlayGraphic = VideoPlaybackPannel->CreateGraphics();
        m_videoPlayGraphic->Clear(BackColor);

        if(orgPlayStat == PS_PLAY) // resume play
            m_CPlayer->PlayerStart();
    }
}

System::Void Form1::VideoBitratePannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
}

System::Void Form1::VideoBitratePicBox_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    if(m_CPlayer)
    {
        Graphics^ g = VideoBitRatePicBox->CreateGraphics();
        Int32 xStart = VideoBitratePannel->HorizontalScroll->Value / m_oscBitRate->mGridWidth * m_oscBitRate->mGridWidth;
        int   xIdx = xStart / m_oscBitRate->mGridWidth;

        int   numPoints = (m_CBitRateStat->getNewstAIdx() - xIdx) < 0? 0: min(m_CBitRateStat->getNewstAIdx() - xIdx, VideoBitratePannel->Width / m_oscBitRate->mGridWidth + 2);
        drawGrid(g, xStart, VideoBitratePannel->Width + 2 * m_oscBitRate->mGridWidth, VideoBitRatePicBox->Height, 20, 0, 0);
        m_oscBitRate->showPoints(m_CBitRateStat->getArray(), max(0, xIdx - 1), numPoints + 1);
        delete g;
    }
}

System::Void Form1::VBVBufferPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
    Graphics^ g = VBVBufferPannel->CreateGraphics();
    g->Clear(BackColor);
    drawGrid(g, 0, VBVBufferPannel->Width, VBVBufferPannel->Height, 20, 20, 5);
    delete g;
}

System::Void Form1::setRenderArea()
{
    Int32     picWidth = m_CPlayer->m_iWidth;
    Int32    picHeight = m_CPlayer->m_iHeight;
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

    ScaledHeight = ((ScaledHeight >> 4) << 4);
    ScaledWidth  = ((ScaledWidth >> 4) << 4);

    m_renderTlx = (pannelWidth - ScaledWidth) >> 1;
    m_renderTly = (pannelHeight - ScaledHeight) >> 1;
    m_renderAreaWidth  = ScaledWidth;
    m_renderAreaHeight = ScaledHeight;
    va_log(LOGLEVEL_INFO, "setRenderArea: VideoRenderWindiw Size (%4d, %4d); RenderArea: topleft = (%4d, %4d), Size = (%4d, %4d)\n", pannelWidth, pannelHeight, m_renderTlx, m_renderTly, m_renderAreaWidth, m_renderAreaHeight);
}

System::Void Form1::drawGrid(Graphics^ g, Int32 xOffset, Int32 Width, Int32 Height, Int32 GridSize, Int32 ipadx, Int32 ipady)
{
    Pen^ pen = gcnew Pen(Color::Gray, 1.0);
    Point^ tl = gcnew Point(); // top left position of drawing area
    tl->X = xOffset + ipadx;
    tl->Y = ipady;
    Point^ br = gcnew Point(); // bottom right position of drawing area
    br->X = xOffset + ipadx + (Width - ipadx) / GridSize * GridSize;
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

    g->DrawLine(pen, tl->X, tl->Y, bl->X, bl->Y);
    g->DrawLine(pen, tr->X, tr->Y, br->X, br->Y);

    pen->Width = 2.0;
    pen->DashStyle = DashStyle::Solid;
    g->DrawLine(pen, tl->X, tl->Y, tr->X, tr->Y);
    g->DrawLine(pen, bl->X, bl->Y, br->X, br->Y);
}

System::Void RenderThreadProc(Object^ data) // render thread calls
{
    Form1^ mainForm = (Form1^)data;
    VideoPlayer* pl = mainForm->m_CPlayer;

    while(pl == NULL) // wait until 
        Sleep(1);

    while(1)
    {
        if(pl->WaitToPlayOrExit() == PS_EXIT)
            break;

        Frame* renderFrame = picture_queue_get_read_picture(&(pl->m_pictq));
        if(!renderFrame)
            return;
        int    rgbFrmWidth = renderFrame->rgbframe->width;
        int   rgbFrmHeight = renderFrame->rgbframe->height;

        mainForm->updateBitStat(renderFrame->frame_pkt_bits, (int)(renderFrame->pts * 1000));

        if(/*!m_doscale || */renderFrame->b_rgbready == false)
        {
            va_log(LOGLEVEL_INFO, "Render frame pts = %8d ms, sws_scale width, height = %d, %d\n", (int)(1000 * renderFrame->pts), renderFrame->rgbframe->width, renderFrame->rgbframe->height);
            sws_scale(pl->m_pSwsCtx, 
                      renderFrame->yuvframe->data, renderFrame->yuvframe->linesize, 0, renderFrame->yuvframe->height, 
                      renderFrame->rgbframe->data, renderFrame->rgbframe->linesize);
        }

        Drawing::Rectangle rect = Drawing::Rectangle(0, 0, rgbFrmWidth, rgbFrmHeight);
        BitmapData^ bmpData = mainForm->m_rpic->LockBits(rect, ImageLockMode::ReadWrite, mainForm->m_rpic->PixelFormat);
        IntPtr   bmpDataPtr = bmpData->Scan0;
        int           bytes = Math::Abs(bmpData->Stride) * mainForm->m_rpic->Height;

        char* p = (char *)bmpDataPtr.ToPointer();
        memcpy(p, renderFrame->rgbframe->data[0], bytes * sizeof(char));

        mainForm->m_rpic->UnlockBits(bmpData);

        va_log(LOGLEVEL_INFO, "Render Frame(pts = %6dms) Started, frameSize = (%4d, %4d), RenderArea: topleft = (%4d, %4d), RenderSize = (%4d, %4d)\n", (int)(1000 * renderFrame->pts), rgbFrmWidth, rgbFrmHeight, mainForm->m_renderTlx, mainForm->m_renderTly,mainForm->m_renderAreaWidth, mainForm->m_renderAreaHeight);
        mainForm->m_videoPlayGraphic->DrawImage(mainForm->m_rpic, mainForm->m_renderTlx, mainForm->m_renderTly, mainForm->m_renderAreaWidth, mainForm->m_renderAreaHeight);
        picture_queue_finish_read(&(pl->m_pictq));
        va_log(LOGLEVEL_INFO, "Render Frame(pts = %6dms) Ended\n", (int)(1000 * renderFrame->pts));
    } 
}

System::Void Form1::openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if(m_CPlayer)
    {
        m_CPlayer->PlayerExit();
        rendThread->Join();  // wait render thread exit first and then delete player
        delete m_CPlayer;
    }

    openFileDialog->ShowDialog();
    mfilename = openFileDialog->FileName;
    this->Text = L"VideoAnalyzer " + mfilename;
    va_log(LOGLEVEL_INFO, "Open File %s\n", (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(mfilename));

    m_CPlayer = new VideoPlayer((char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(mfilename));
    m_CPlayer->PlayerInit();
    rendThread = gcnew Thread(gcnew ParameterizedThreadStart(&RenderThreadProc));
    rendThread->Start(this);

    mVideoInfo = System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)m_CPlayer->GetVideoInfo());
    setVideoInfoMethod(mVideoInfo);  // show basic video infos

    setRenderArea();

    if(m_doscale)
    {
        m_rpic = gcnew Bitmap(m_renderAreaWidth, m_renderAreaHeight, PixelFormat::Format24bppRgb);
        m_CPlayer->InitScaleParameters(m_renderAreaWidth, m_renderAreaHeight);
    }
    else
    {
        m_rpic = gcnew Bitmap(m_CPlayer->m_iWidth, m_CPlayer->m_iHeight, PixelFormat::Format24bppRgb);
        m_CPlayer->InitScaleParameters(m_CPlayer->m_iWidth, m_CPlayer->m_iHeight);
    }

    int duration_sec = m_CPlayer->GetDuration() + 1;
    VideoBitRatePicBox->Width = m_oscBitRate->mGridWidth * duration_sec;
    //m_CBitRateStat->updateLastPts((int)1000 * (m_pl->avftx->start_time) / AV_TIME_BASE);
    if(m_CPlayer->GetFileBitRate())
    {
        m_oscBitRate->setYMax((int)(m_CPlayer->GetFileBitRate() / 1000) * 2);
        m_oscBitRate->setYScale((float)m_oscBitRate->mactHeight / m_oscBitRate->mYMax);
    }
    m_oscBitRate->reset();
    m_CBitRateStat->reset();
    m_CFrameBitsStat->reset();

    m_videoPlayGraphic->Clear(BackColor);
    Graphics^ g = VideoBitRatePicBox->CreateGraphics();
    g->Clear(BackColor);
    delete g;
    setBRPannelHScrollMethod(0);
}

System::Void Form1::StopButton_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if(m_CPlayer)
        m_CPlayer->PlayerPause();
}

System::Void Form1::updateBitStat(int frameBits, int pts)
{
    m_CFrameBitsStat->appendItem(frameBits, pts);
    m_CBitRateStat->accumItem(frameBits, pts);
    if(m_CBitRateStat->getAccumInterval() >= 1000) // 1s
    {
        Graphics^ g = VideoBitRatePicBox->CreateGraphics();
        Int32 xStart = VideoBitratePannel->HorizontalScroll->Value / m_oscBitRate->mGridWidth * m_oscBitRate->mGridWidth;
        int   xIdx = xStart / m_oscBitRate->mGridWidth;
        drawGrid(g, xStart, VideoBitratePannel->Width + 2* m_oscBitRate->mGridWidth, VideoBitRatePicBox->Height, 20, 0, 0);

        int xOffset = VideoBitratePannel->HorizontalScroll->Value;
        int targetX = m_CBitRateStat->getNewstAIdx() * m_oscBitRate->mGridWidth;
        if(targetX - xOffset > VideoBitratePannel->Width) // reached right boundary, need to scroll left 
        {
            Invoke(msetBitRatePannelHScrollDelegate, targetX - VideoBitratePannel->Width / 2); 
            System::Windows::Forms::PaintEventArgs^  e;
            Graphics^ g = VideoBitRatePicBox->CreateGraphics();
            g->Clear(BackColor);
            delete g;
            VideoBitratePicBox_Paint(this, e);
        }
        m_oscBitRate->addPoint(m_CBitRateStat->getNewstAIdx(), m_CBitRateStat->getNewstValue() / 1000); // bitrate(kbps)
        m_CBitRateStat->updateLastPts(pts);
        m_CBitRateStat->incArrIdx();
    }
}

System::Void Form1::PlayButton_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if(String::IsNullOrEmpty(mfilename)) // input file not choosen yet
        return;

    if(m_CPlayer)
    {
        m_CPlayer->PlayerStart();
        va_log(LOGLEVEL_INFO, "Play Button Clicked, start to play.\n");
    }
}