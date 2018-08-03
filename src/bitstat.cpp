#include "StdAfx.h"
#include "bitstat.h"
#include<stdlib.h>
#include<memory.h>

BitStat::BitStat(void)
{
    m_iBitAIdx   = 0;     // Array Index
    m_iCurPts    = 0;
    m_iBitASize  = 1000;  // bit rate array size
    m_piBitArray = (int *)malloc(sizeof(int) * m_iBitASize);
    if(m_piBitArray != NULL)
        memset(m_piBitArray, 0, sizeof(int) * m_iBitASize);
}

BitStat::BitStat(int iBitASize)
{
    m_iBitAIdx   = 0;     // Array Index
    m_iCurPts    = 0;
    m_iBitASize  = iBitASize;  // bit rate array size
    m_piBitArray = (int *)malloc(sizeof(int) * m_iBitASize);
    if(m_piBitArray != NULL)
        memset(m_piBitArray, 0, sizeof(int) * m_iBitASize);
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
        int  iNewArrSize = 2 * m_iBitASize;
        int*     pNewArr = NULL;
        pNewArr = (int *)malloc(sizeof(int) * iNewArrSize); // increase array size
        if(pNewArr == NULL)
            return -1;

        memcpy(pNewArr, m_piBitArray, sizeof(int) * m_iBitASize);
        memset(pNewArr + m_iBitASize, 0, sizeof(int) * (iNewArrSize - m_iBitASize));
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
        memset(m_piBitArray, 0, sizeof(int) * m_iBitASize);
}

void BitStat::appendItem(int value)
{
    if(reallocBitArrayIfNeeded() >= 0)
        m_piBitArray[m_iBitAIdx++] = value;
}

void BitStat::appendItem(int value, int pts)
{
    if(reallocBitArrayIfNeeded() >= 0)
    {
        m_piBitArray[m_iBitAIdx++] = value;
        m_iCurPts = pts;
    }
}

void BitStat::accumItem(int value)
{
    m_piBitArray[m_iBitAIdx] += value;
}

void BitStat::accumItem(int value, int pts)
{
    m_piBitArray[m_iBitAIdx] += value;
    m_iCurPts = pts;
}

void BitStat::incArrIdx()
{
    m_iBitAIdx++;
    reallocBitArrayIfNeeded();
}

void BitStat::updateLastPts(int pts)
{
    m_iLastPts = pts;
}

int BitStat::getAccumInterval(void)
{
    return (m_iCurPts - m_iLastPts);
}

int BitStat::getNewstValue(void)
{
    return m_piBitArray[m_iBitAIdx];
}

int BitStat::getNewstAIdx(void)
{
    return m_iBitAIdx;
}

int* BitStat::getArray(void)
{
    return m_piBitArray;
}