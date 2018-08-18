#include "StdAfx.h"
#include "oscillogram.h"
#include <stdio.h>

#define str2String(str) System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(char*)(str))

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

void convertPtsToTime(char* time, int pts) // pts = ms
{
    int secs = (int)pts / 1000;
    int mins = secs / 60;
    int hour = mins / 60;
    secs %= 60;
    mins %= 60;
    sprintf_s(time, 20, "%02d:%02d:%02d", hour, mins, secs);
}

void oscillogram::ShowNewPoint()
{
    int xValue = m_CBitStat->getNewstAIdx();
    int yValue = m_CBitStat->getNewstValue();
    bool showlegend = m_CBitStat->getPointByIdx(xValue)->b_showlegend;
    int  pts = m_CBitStat->getPointByIdx(xValue)->pts; // ms
    int xPos = xValue * mGridWidth;
    int yPos = mBlP.Y - (int)(yValue * mYScale);
    char time[20];

    pthread_mutex_lock(m_mtx);
    if(mLastP.Y > 0)
    {
        mGraphic = mPicBox->CreateGraphics();
        mGraphic->DrawLine(mPen, mLastP.X, mLastP.Y, xPos, yPos);
        if(showlegend)
        {
            convertPtsToTime(time, pts - m_CBitStat->getFirstPts());
            mGraphic->DrawString(str2String(time), mFont, Brushes::Black, xPos, mBlP.Y);
        }
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

void oscillogram::drawYLengend(int xStart)
{
    int xPos = xStart * mGridWidth;
    char str[20];
    int yLengendValue;
    mGraphic = mPicBox->CreateGraphics();
    for(int y = mBlP.Y; y >= 0; y = y - 2 * mGridWidth)
    {
        yLengendValue = (int)((float)(mBlP.Y - y) / mYScale);
        sprintf_s(str, 20, "%d", yLengendValue);
        mGraphic->DrawString(str2String(str), mFont, Brushes::Black, xPos, y);
    }
    delete mGraphic;
}

void oscillogram::showPoints(int xStart, int numPoints)
{
    int cnt = 0;
    int xPos, yPos;
    int xPosOffset = xStart * mGridWidth;
    BitPoint* bp;
    char time[20];

    pthread_mutex_lock(m_mtx);
    mLastP.Y = 0;
    mGraphic = mPicBox->CreateGraphics();
    //mGraphic->Clear(Color::White);

    while(cnt < numPoints)
    {
        bp = m_CBitStat->getPointByIdx(xStart + cnt);
        xPos = xPosOffset + cnt * mGridWidth;
        yPos = mBlP.Y - (int)(bp->y * mYScale);

        if(mLastP.Y > 0)
        {
            mGraphic->DrawLine(mPen, mLastP.X, mLastP.Y, xPos, yPos);
            if(bp->b_showlegend)
            {
                convertPtsToTime(time, bp->pts - m_CBitStat->getFirstPts());
                mGraphic->DrawString(str2String(time), mFont, Brushes::Black, xPos, mBlP.Y);
            }
        }

        mLastP.X = xPos;
        mLastP.Y = yPos;
        cnt++;
    }
    delete mGraphic;
    pthread_mutex_unlock(m_mtx);
}