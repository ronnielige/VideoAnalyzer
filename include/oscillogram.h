#ifndef _OSCILLOGRAM_H_
#define _OSCILLOGRAM_H_

#include "w32thread.h"
#include "bitstat.h"
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
    Font^       mFont;
    int       mGraWidth;
    int       mGraHeight;
    Point     mBlP;    // bottom left point of image
    Point     mTlP;    // top left point of image
    Point     mBrP;    // bottom right point of image
    Point     mLastP;  // last draw point

    int       mGridWidth;
    int       mYMax;
    int       mactHeight;
    float     mYScale;

    pthread_mutex_t* m_mtx;

public:
    BitStat*    m_CBitStat;   // stat bits per second or stat frame bit

    oscillogram(PictureBox^ picb);
    ~oscillogram();

    void AddPoint(int yvalue, int pts);
    void ShowNewPoint(void);
    void drawYLengend(int xStart);
    void showPoints(int xStart, int numPoints);
    void SetFont(Font^ fnt)
    {
        mFont = fnt;
    }
    void setYMax(int ym)
    {
        mYMax = ym;
    }
    void setYScale(float ys)
    {
        mYScale = ys;
    }
    void reset();
    void setcoordinate(int tlx, int tly, int blx, int bly);
    bool bYOutofBound(int yValue);
};

#endif