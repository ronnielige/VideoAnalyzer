#include "StdAfx.h"
#include "oscillogram.h"

oscillogram::oscillogram(PictureBox^ picb, int w, int h)
{
    mPicBox     = picb;
    m_mtx       = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    mPen        = gcnew Pen(Color::Red, 2.0);
    pthread_mutex_init(m_mtx, NULL);
    mGraWidth  = w;
    mGraHeight = h;
    mGridWidth = 20;

    mTlP.X = 0;  
    mTlP.Y = 0;

    mBlP.X = 0;
    mBlP.Y = mGraHeight / mGridWidth * mGridWidth; 
    mactHeight = mBlP.Y - mTlP.Y;

    mBrP.X = mGraWidth / mGridWidth * mGridWidth;
    mBrP.Y = mBlP.Y;

    mLastP.X = 0;
    mLastP.Y = 0;

    mYMax = 10000;
    mYScale = (float)mactHeight / mYMax;
    //mPen->DashStyle = DashStyle::DashStyleSolid;
}

oscillogram::~oscillogram()
{
    pthread_mutex_destroy(m_mtx);
}

void oscillogram::reset()
{
    mLastP.X = 0;
    mLastP.Y = 0;
}

void oscillogram::addPoint(int xValue, int yValue)
{
    int xPos = xValue * mGridWidth;
    int yPos = mBlP.Y - (int)(yValue * mYScale);

    pthread_mutex_lock(m_mtx);
    if(mLastP.Y > 0)
    {
        mGraphic = mPicBox->CreateGraphics();
        mGraphic->DrawLine(mPen, mLastP.X, mLastP.Y, xPos, yPos);
        delete mGraphic;
    }
    mLastP.X = xPos;
    mLastP.Y = yPos;
    pthread_mutex_unlock(m_mtx);
}

void oscillogram::showPoints(int* yArray, int xStart, int numPoints)
{
    int cnt = 0;
    int xPos, yPos;
    int xPosOffset = xStart * mGridWidth;

    pthread_mutex_lock(m_mtx);
    mLastP.Y = 0;
    mGraphic = mPicBox->CreateGraphics();
    //mGraphic->Clear(Color::White);

    while(cnt < numPoints)
    {
        xPos = xPosOffset + cnt * mGridWidth;
        yPos = mBlP.Y - (int)(yArray[xStart + cnt] * mYScale / 1000);

        if(mLastP.Y > 0)
            mGraphic->DrawLine(mPen, mLastP.X, mLastP.Y, xPos, yPos);

        mLastP.X = xPos;
        mLastP.Y = yPos;
        cnt++;
    }
    delete mGraphic;
    pthread_mutex_unlock(m_mtx);
}