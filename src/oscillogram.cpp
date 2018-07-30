#include "StdAfx.h"
#include "oscillogram.h"

oscillogram::oscillogram(PictureBox^ picb, int w, int h)
{
    int actHeight = 0;
    mPicBox     = picb;
    mGraphic   = mPicBox->CreateGraphics();
    mPen       = gcnew Pen(Color::Red, 1.0);
    mGraWidth  = w;
    mGraHeight = h;
    mGridWidth = 20;

    mTlP.X = 0;  
    mTlP.Y = 0;

    mBlP.X = 0;
    mBlP.Y = mGraHeight / mGridWidth * mGridWidth; 
    actHeight = mBlP.Y - mTlP.Y;

    mBrP.X = mGraWidth / mGridWidth * mGridWidth;
    mBrP.Y = mBlP.Y;

    mLastP.X = 0;
    mLastP.Y = 0;

    mYMax = 10000;
    mYScale = (float)actHeight / mYMax;
    mPen->DashStyle = DashStyle::DashDot;
}

oscillogram::~oscillogram()
{
    delete mGraphic;
}

void oscillogram::addPoint(int xValue, int yValue)
{
    int xPos = xValue * mGridWidth;
    int yPos = (int)(yValue * mYScale);
    mGraphic->DrawLine(mPen, mLastP.X, mLastP.Y, xPos, yPos);
    mLastP.X = xPos;
    mLastP.Y = yPos;
}