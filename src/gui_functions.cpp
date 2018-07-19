//Implement gui control functions
#include "StdAfx.h"
#include "Form1.h"

using namespace VideoAnalyzer;

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
    PlayStat = PS_PLAY;  // start or resume play
    Graphics^ g = VideoPlaybackPannel->CreateGraphics();
    g->Clear(Color::White);
    Bitmap^ pic = gcnew Bitmap(mfilename);
    Int32 picWidth = pic->Width, picHeight = pic->Height;
    char res[20]; 
    sprintf(res, "%d x %d", picWidth, picHeight);
    this->resolutionLabel->Text = System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)res);
    //Bitmap^ newpic = gcnew Bitmap(picWidth, picHeight, PixelFormat::Format24bppRgb);
    System::Drawing::Rectangle rect = System::Drawing::Rectangle(0, 0, picWidth, picHeight);
    Bitmap^ newpic = pic->Clone(rect, PixelFormat::Format24bppRgb);
    BitmapData^ bmpData = newpic->LockBits(rect, ImageLockMode::ReadWrite, newpic->PixelFormat);
    IntPtr ptr = bmpData->Scan0;
    Int32 cnt;
    int bytes = Math::Abs(bmpData->Stride) * newpic->Height;
    if(0 /* Method 1: */)
    {
        array<Byte>^ rgbValues = gcnew array<Byte>(bytes);
        for(cnt = 0; cnt < rgbValues->Length; cnt += 3)
        {
            rgbValues[cnt] = 87;
            rgbValues[cnt + 1] = 055;
            rgbValues[cnt + 2] = 253;
        }
        System::Runtime::InteropServices::Marshal::Copy(rgbValues, 0, ptr, bytes);
    }

    if(1 /* Method 2: directly operate bmpData */)
    {
        char* p = (char *)ptr.ToPointer();
        for(cnt = 0; cnt < bytes; cnt += 3)
        {
            //p[cnt] += 40;      // blue
            //p[cnt + 1] = 9;  // green
            //p[cnt + 2] += 255; // red
        }
    }
    newpic->UnlockBits(bmpData);
    showFrame(g, VideoPlaybackPannel->Width, VideoPlaybackPannel->Height, newpic);

    delete g;
}