#ifndef _BITSTAT_H_
#define _BITSTAT_H_

class BitStat{
private:
    int* m_piBitArray;   // int array, each item represents bits of a frame or bitrate of a second
    int  m_iBitAIdx;
    int  m_iBitASize;
    int  m_iCurPts;
    int  m_iLastPts;
    int  reallocBitArrayIfNeeded(void);

public:
    BitStat();
    BitStat(int iBitASize);
    ~BitStat();

    void reset(void);
    void appendItem(int value);           // just add a new item to tail of array
    void appendItem(int value, int pts);
    void accumItem(int value);            // accumulate value to last item of array 
    void accumItem(int value, int pts);
    void incArrIdx(void);
    void updateLastPts(int pts);
    int  getAccumInterval();              // get accumulation interval 

    int  getNewstValue(void);
    int  getNewstAIdx(void);

    int* getArray();
};

#endif