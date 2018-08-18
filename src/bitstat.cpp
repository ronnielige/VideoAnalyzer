#include "StdAfx.h"
#include "bitstat.h"
#include<stdlib.h>
#include<memory.h>

BitStat::BitStat(void)
{
    m_iBitAIdx   = 0;     // Array Index
    m_iCurPts    = 0;
    m_iBitASize  = 1000;  // bit rate array size
    m_piBitArray = (BitPoint *)malloc(sizeof(BitPoint) * m_iBitASize);
    if(m_piBitArray != NULL)
        memset(m_piBitArray, 0, sizeof(BitPoint) * m_iBitASize);
}

BitStat::BitStat(int iBitASize)
{
    m_iBitAIdx   = 0;     // Array Index
    m_iCurPts    = 0;
    m_iBitASize  = iBitASize;  // bit rate array size
    m_piBitArray = (BitPoint *)malloc(sizeof(BitPoint) * m_iBitASize);
    if(m_piBitArray != NULL)
        memset(m_piBitArray, 0, sizeof(BitPoint) * m_iBitASize);
}

BitStat::~BitStat(void)
{
    if(m_piBitArray)
        free(m_piBitArray);
}

int BitStat::reallocBitArrayIfNeeded(void)
{
    if(m_iBitAIdx >= m_iBitASize && m_piBitArray)
    {
        int   iNewArrSize = 2 * m_iBitASize;
        BitPoint* pNewArr = NULL;
        pNewArr = (BitPoint *)malloc(sizeof(BitPoint) * iNewArrSize); // increase array size
        if(pNewArr == NULL)
            return -1;

        memcpy(pNewArr, m_piBitArray, sizeof(BitPoint) * m_iBitASize);
        memset(pNewArr + m_iBitASize, 0, sizeof(BitPoint) * (iNewArrSize - m_iBitASize));
        free(m_piBitArray);

        m_piBitArray = pNewArr;
        m_iBitASize  = iNewArrSize;
    }
    return 0;
}

void BitStat::reset(void)
{
    m_iBitAIdx = 0;
    m_iLastPts = 0;
    if(m_piBitArray != NULL)
        memset(m_piBitArray, 0, sizeof(BitPoint) * m_iBitASize);
}

void BitStat::appendItem(int value)
{
    if(reallocBitArrayIfNeeded() >= 0)
    {
        m_piBitArray[m_iBitAIdx].x = m_iBitAIdx;
        m_piBitArray[m_iBitAIdx].y = value;
        m_iBitAIdx++;
    }
}

void BitStat::appendItem(int value, int pts)
{
    if(reallocBitArrayIfNeeded() >= 0)
    {
        m_piBitArray[m_iBitAIdx].x   = m_iBitAIdx;
        m_piBitArray[m_iBitAIdx].y   = value;
        m_piBitArray[m_iBitAIdx].pts = pts;
        m_iBitAIdx++;
    }
}

int BitStat::getNewstValue(void)
{
    return m_piBitArray[m_iBitAIdx - 1].y;
}

int BitStat::getNewstAIdx(void)
{
    return m_iBitAIdx - 1;
}

int BitStat::getNewstPts(void)
{
    if(m_iBitAIdx)
        return m_piBitArray[m_iBitAIdx - 1].pts;
    else
        return m_piBitArray[0].pts;
}

void BitStat::setFirstPts(int pts)
{
    m_piBitArray[0].pts = pts;
}

BitPoint* BitStat::getPointByIdx(int idx)
{
    return &m_piBitArray[idx];
}