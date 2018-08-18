#include "StdAfx.h"
#include "oscillogram.h"

oscillogram::oscillogram(PictureBox^ picb)
{
    mPicBox     = picb;
    m_mtx       = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    mPen        = gcnew Pen(Color::Red, 2.0);
    pthread_mutex_init(m_mtx, NULL);
    mGridWidth = 20;

    mTlP.X = 0;  
    mTlP.Y = 0;

    mBlP.X = 0;
    mBlP.Y = picb->Height / mGridWidth * mGridWidth; 
    mactHeight = mBlP.Y - mTlP.Y;

    mBrP.X = picb->Width / mGridWidth * mGridWidth;
    mBrP.Y = mBlP.Y;

    mLastP.X = 0;
    mLastP.Y = 0;

    mYMax = 10000;
    mYScale = (float)mactHeight / mYMax;

    m_CBitStat   = new BitStat(10000);
    //mPen->DashStyle = DashStyle::DashStyleSolid;
}

void oscillogram::setcoordinate(int tlx, int tly, int blx, int bly)
{
    mTlP.X = tlx;
    mTlP.Y = tly;
    mBlP.X = blx;
    mBlP.Y = bly;
    mactHeight = mBlP.Y - mTlP.Y;
    setYScale((float)mactHeight / mYMax);
}

oscillogram::~oscillogram()
{
    pthread_mutex_destroy(m_mtx);
    if(m_CBitStat)
        delete m_CBitStat;
}

void oscillogram::reset()
{
    mLastP.X = 0;
    mLastP.Y = 0;
    m_CBitStat->reset();
}

void oscillogram::AddPoint(int yvalue, int pts)
{
    m_CBitStat->appendItem(yvalue, pts);
}

void oscillogram::ShowNewPoint()
{
    int xValue = m_CBitStat->getNewstAIdx();
    int yValue = m_CBitStat->getNewstValue();
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

bool oscillogram::bYOutofBound(int yValue)
{
    int yPos = mBlP.Y - (int)(yValue * mYScale);
    if(yPos < 0)
        return true;
    else
        return false;
}

void oscillogram::showPoints(int xStart, int numPoints)
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
        yPos = mBlP.Y - (int)(m_CBitStat->getPointByIdx(xStart + cnt)->y * mYScale);

        if(mLastP.Y > 0)
            mGraphic->DrawLine(mPen, mLastP.X, mLastP.Y, xPos, yPos);

        mLastP.X = xPos;
        mLastP.Y = yPos;
        cnt++;
    }
    delete mGraphic;
    pthread_mutex_unlock(m_mtx);
}