//Implement gui control functions
#include "StdAfx.h"
#include "Form1.h"
#include "player.h"
using namespace VideoAnalyzer;

Form1::Form1(void)
{
    InitializeComponent();
    PlayStat = PS_NONE;    // init Player Stat 
    m_mtxPlayStat = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    m_condPlayCond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(m_mtxPlayStat, NULL);
    pthread_cond_init(m_condPlayCond, NULL);

    readThread = gcnew Thread(gcnew ParameterizedThreadStart(&readThreadProc));
    readThread->Start(this);

    decThread = gcnew Thread(gcnew ParameterizedThreadStart(&decodeThreadProc));
    decThread->Start(this);

    rendThread = gcnew Thread(gcnew ParameterizedThreadStart(&renderThreadProc));
    rendThread->Start(this);

    mSetVidInfDelegate = gcnew setVideoInfo(this, &Form1::setVideoInfoMethod);
}

Form1::~Form1()
{
    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_EXIT;
    pthread_cond_broadcast(m_condPlayCond);  // send exit to threads
    pthread_mutex_unlock(m_mtxPlayStat);
    readThread->Join();   // wait read   thread to finish
    decThread->Join();    // wait decode thread to finish
    rendThread->Join();   // wait render thread to finish

    pthread_mutex_destroy(m_mtxPlayStat);
    pthread_cond_destroy(m_condPlayCond);
    free(m_mtxPlayStat);
    free(m_condPlayCond);

    if (components)
    {
        delete components;
    }
}

System::Void Form1::VideoPlaybackPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
{
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

System::Void Form1::showFrame(Graphics^ g, Int32 pannelWidth, Int32 pannelHeight, Bitmap^ pic)
{
    float ScaleY = (float)pannelHeight / pic->Height; 
    float ScaleX = (float)pannelWidth  / pic->Width; 
    Int32 ScaledHeight = pannelHeight;
    Int32 ScaledWidth = pic->Width * ScaleY;
    Int32 tl_x, tl_y, br_x, br_y;
    if(ScaledWidth > pannelWidth)
    {
        ScaledWidth  = pannelWidth;
        ScaledHeight = pic->Height * ScaleX;
    }
    tl_x = (pannelWidth - ScaledWidth) / 2;
    br_x = (pannelWidth + ScaledWidth) / 2;
    tl_y = (pannelHeight - ScaledHeight) / 2;
    br_y = (pannelHeight + ScaledHeight) / 2;
    g->DrawImage(pic, tl_x, tl_y, ScaledWidth, ScaledHeight);
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
    openFileDialog1->ShowDialog();
    mfilename = openFileDialog1->FileName;
    this->Text = L"VideoAnalyzer " + mfilename;
}

System::Void Form1::StopButton_Click(System::Object^  sender, System::EventArgs^  e) 
{
    PlayStat = PS_PAUSE;  // Stop play
    Graphics^ g = VideoPlaybackPannel->CreateGraphics();
    g->Clear(Color::White);
    delete g;
}

System::Void Form1::PlayButton_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if(String::IsNullOrEmpty(mfilename)) // input file not choosen yet
        return;

    pthread_mutex_lock(m_mtxPlayStat);
    PlayStat = PS_PLAY;
    pthread_cond_broadcast(m_condPlayCond);  // send exit to threads
    pthread_mutex_unlock(m_mtxPlayStat);

    //Graphics^ g = VideoPlaybackPannel->CreateGraphics();
    //g->Clear(Color::White);
    //Bitmap^ pic = gcnew Bitmap(mfilename);
    //Int32 picWidth = pic->Width, picHeight = pic->Height;
    //char res[20]; 
    //sprintf(res, "%d x %d", picWidth, picHeight);
    //VideoInfoLabel->Text += "\nResolution:\n    " + System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)res);
    ////Bitmap^ newpic = gcnew Bitmap(picWidth, picHeight, PixelFormat::Format24bppRgb);
    //System::Drawing::Rectangle rect = System::Drawing::Rectangle(0, 0, picWidth, picHeight);
    //Bitmap^ newpic = pic->Clone(rect, PixelFormat::Format24bppRgb);
    //BitmapData^ bmpData = newpic->LockBits(rect, ImageLockMode::ReadWrite, newpic->PixelFormat);
    //IntPtr ptr = bmpData->Scan0;
    //Int32 cnt;
    //int bytes = Math::Abs(bmpData->Stride) * newpic->Height;
    //if(0 /* Method 1: */)
    //{
    //    array<Byte>^ rgbValues = gcnew array<Byte>(bytes);
    //    for(cnt = 0; cnt < rgbValues->Length; cnt += 3)
    //    {
    //        rgbValues[cnt] = 87;
    //        rgbValues[cnt + 1] = 055;
    //        rgbValues[cnt + 2] = 253;
    //    }
    //    System::Runtime::InteropServices::Marshal::Copy(rgbValues, 0, ptr, bytes);
    //}

    //if(1 /* Method 2: directly operate bmpData */)
    //{
    //    char* p = (char *)ptr.ToPointer();
    //    for(cnt = 0; cnt < bytes; cnt += 3)
    //    {
    //        //p[cnt] += 40;      // blue
    //        //p[cnt + 1] = 9;  // green
    //        //p[cnt + 2] += 255; // red
    //    }
    //}
    //newpic->UnlockBits(bmpData);
    //showFrame(g, VideoPlaybackPannel->Width, VideoPlaybackPannel->Height, newpic);

    //delete g;
}