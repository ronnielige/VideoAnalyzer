#ifndef _OSCILLOGRAM_H_
#define _OSCILLOGRAM_H_

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;
using namespace System::Drawing::Imaging;

ref class oscillogram 
{
public:
    PictureBox^ mPicBox;
    Graphics^   mGraphic;
    Pen^        mPen;
    int       mGraWidth;
    int       mGraHeight;
    Point     mBlP;    // bottom left point of image
    Point     mTlP;    // top left point of image
    Point     mBrP;    // bottom right point of image
    Point     mLastP;  // last draw point

    int       mGridWidth;
    int       mYMax;
    float     mYScale;

public:
    oscillogram(PictureBox^ picb, int w, int h);
    ~oscillogram();

    void addPoint(int xvalue, int yvalue);
    void increadPixBoxWidth(int w);
};

#endif