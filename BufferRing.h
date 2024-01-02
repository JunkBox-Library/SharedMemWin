
#ifndef  __BUFFER_RING_H__
#define  __BUFFER_RING_H__

#pragma once


#include "tools++.h"


// kindData
#define  UNKNOWN_DATA       0
#define  BINARY_DATA        1
#define  TEXT_DATA          2
#define  TEXT_HALFWAY       3
#define  BINHEX_DATA        4

// inputSrc
#define  OPERATION_DATA     0
#define  SERVER_DATA        1
#define  CLIENT_DATA        2
#define  PROXY_DATA         3


class CBufferRing
{
public:
    CBufferRing(int size) { init(size);}
    ~CBufferRing(void);

public:
    int     init(int size);
    void    clear(void);
    void    putBufferRing(Buffer buf, int input, int kind=BINHEX_DATA);
    void    rewriteBinHexBufferRing(int n, int input);

    Buffer  getBufferRing(void);
    Buffer  getBufferRing(int pos);

    int     getMaxBufSize(void)      { return maxBufSize;}
    int     getMaxLineX(void)        { return maxLineX;}
    int     getMaxLineY(void)        { return maxLineY;}
    int     getTotalSize(void)       { return tlDataSize;}
    int     getLastPosition(void)    { return wPos;}

    int     getLengthX(int n)        { if(n<0) n+=maxBufSize; return (int)strlen((const char*)pBuf[n%maxBufSize].buf);}
    int     getKindData(int n)       { if(n<0) n+=maxBufSize; return kindData[n%maxBufSize];}

public:
    Buffer* pBuf;

protected:
    int     maxBufSize;      // Buffer の数．ラインの数．
    int     tlDataSize;

    int     maxLineX;        // コンテキストのXサイズ
    int     maxLineY;        // コンテキストのYサイズ
    int     indentSize;      // 

    int     rPos;
    int     wPos;

    int*    kindData;        //
    int*    inputSrc;        //
};


#endif
