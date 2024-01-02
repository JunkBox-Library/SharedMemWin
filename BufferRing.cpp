

#include "stdafx.h"
#include "BufferRing.h"

using namespace jbxl;


CBufferRing::~CBufferRing(void)
{
    //DEBUG_Error("ディストラクタ：IN  CBufferRing");

    if(pBuf!=NULL) del_Buffer_dim(&pBuf);

    //DEBUG_Error("ディストラクタ：OUT CBufferRing");
}


int  CBufferRing::init(int size)
{
    bool  err_flag = false;

    maxBufSize = tlDataSize = 0;
    maxLineX   = maxLineY   = 0;
    rPos = wPos = 0;
    indentSize = 13;    // 5 + 8  "1234 SERVER: "

    pBuf   = (Buffer*)malloc(sizeof(Buffer)*size);
    kindData = (int*)malloc(sizeof(int)*size);
    inputSrc = (int*)malloc(sizeof(int)*size);

    if (pBuf!=NULL && inputSrc!=NULL && kindData!=NULL) {
        memset(pBuf,      0, sizeof(Buffer)*size);
        memset(kindData, 0, sizeof(int)*size);
        memset(inputSrc, 0, sizeof(int)*size);
        maxBufSize = size;

        for (int i=0; i<size; i++) {
            pBuf[i] = make_Buffer(LBUF);
            if (pBuf[i].buf==NULL) {
                err_flag = true;
                break;
            }
            pBuf[i].state = size;
        }
    }
    else {
        err_flag = true;
    }

    if (err_flag) {
        if (pBuf!=NULL) del_Buffer_dim(&pBuf);
        if (kindData!=NULL) free(kindData);
        if (inputSrc!=NULL) free(inputSrc);
        kindData = inputSrc = NULL;
        return 1;
    }

    return 0;
}


void  CBufferRing::clear()
{
    for (int i=0; i<maxBufSize; i++) {
        pBuf[i].buf[0] = '\0';
        pBuf[i].vldsz  = 0;
    }

    tlDataSize = 0;
    maxLineX = maxLineY = 0;
    rPos = wPos = 0;

    return;
}


void  CBufferRing::putBufferRing(Buffer buf, int input, int kind)
{
    int  i,  nxt = 0;
    bool recalcX = false;
    char num[10];

    kind = BINHEX_DATA;
    if (buf.buf==NULL || buf.vldsz<=0) return;
    Buffer* dim = decompline_Buffer_dim(buf, ON);
    if (dim==NULL || dim->state<=0) return;
    tlDataSize += buf.vldsz;    

#ifdef DEBUG
    int total = 0;
    for (i=0; i<dim->state; i++) {
        total += dim[i].vldsz;
    }
    if (buf.vldsz!=total) {
        DEBUG_ERROR("putBufferRing: ERROR: mismatch total data size!! %d %d", buf.vldsz, total);
    }
#endif

    // 前の行の続き
    int pos = wPos - 1;
    if (pos<0) pos += maxBufSize;
    if (kindData[pos]==TEXT_HALFWAY && isText_Buffer(dim[0]) 
            && inputSrc[pos]==input && input!=OPERATION_DATA && kind==UNKNOWN_DATA) {
        cat_Buffer(&dim[0], &pBuf[pos]);
        kindData[pos] = BINARY_DATA;// TEXT_DATA;
        maxLineX = max(maxLineX, getLengthX(pos));
        nxt = 1;    // 処理済
    }
    
    // 新しい行．
    pos = wPos - 1;    
    for (i=nxt; i<dim->state; i++) {
        pos++;        // == wPos
        if (pos>=maxBufSize) pos -= maxBufSize;
        if (!recalcX && maxLineY==maxBufSize && maxLineX==getLengthX(pos)) recalcX = true;

        sprintf(num, "%04d ", pos);
        copy_s2Buffer(num, &pBuf[pos]);
        if      (input==SERVER_DATA) cat_s2Buffer("SERVER: ", &pBuf[pos]);
        else if (input==CLIENT_DATA) cat_s2Buffer("++++++: ", &pBuf[pos]);
        else if (input==PROXY_DATA)  cat_s2Buffer("PROXY : ", &pBuf[pos]);
        else                         cat_s2Buffer("        ", &pBuf[pos]);

        cat_Buffer(&dim[i], &pBuf[pos]);
        if (kind == UNKNOWN_DATA) kindData[pos] = BINARY_DATA;// TEXT_DATA;
        else                    kindData[pos] = BINARY_DATA;// kind;
        inputSrc[pos] = input;
        maxLineX = max(maxLineX, getLengthX(pos));
    }

    // 最後のデータの再確認
    if (nxt<dim->state && kind==UNKNOWN_DATA) {
        if (isText_Buffer(dim[dim->state-1])) {
            unsigned char end = dim[dim->state-1].buf[dim[dim->state-1].vldsz-1]; 
            if (end != CHAR_CR && end != CHAR_LF) kindData[pos] = BINARY_DATA;// TEXT_HALFWAY;
        }
        else {
            kindData[pos] = BINARY_DATA;
        }
    }

    pos++;
    if (pos>=maxBufSize) pos -= maxBufSize;
    wPos = pos;

    // 次のn行を空行にする．
    if (input!=OPERATION_DATA) {
        Buffer spbuf = rept_Buffer('=', 120);
        for (i=0; i<1; i++) {
            copy_Buffer(&spbuf, &pBuf[pos]);
            inputSrc[pos] = OPERATION_DATA;
            kindData[pos] = BINARY_DATA;// TEXT_DATA;
            pos++;
            if (pos>=maxBufSize) pos -= maxBufSize;
        }
        free_Buffer(&spbuf);
    }

    // コンテキストのサイズを計算
    if (maxLineY<maxBufSize) {
        if (wPos+1<maxLineY) maxLineY = maxBufSize;
        else  maxLineY = wPos + 1;
    }
    //DEBUG_Warning("MaxY = %d", maxLineY);
    
    if (recalcX) {
        maxLineX = 0;
        for (i=0; i<maxBufSize; i++) {
            maxLineX = max(maxLineX, getLengthX(i));
        }
    }

    del_Buffer_dim(&dim);
    return;
}


void  CBufferRing::rewriteBinHexBufferRing(int n, int input)
{
    while (n<0) n += maxBufSize;
    if (n>=maxBufSize) n = n%maxBufSize;

    Buffer dup = pBuf[n];
    dup.buf    = dup.buf   + indentSize;
    dup.vldsz  = dup.vldsz - indentSize;

    Buffer buf = dump_Buffer(dup);
    wPos = n;
    putBufferRing(buf, input, BINHEX_DATA);

    free_Buffer(&buf);
}


Buffer  CBufferRing::getBufferRing(void)
{
    Buffer buf = make_Buffer(LBUF);

    copy_Buffer(&pBuf[rPos], &buf);
    buf.state = inputSrc[rPos];

    rPos++;
    if (rPos>=maxBufSize) rPos -= maxBufSize;

    return buf;
}


Buffer  CBufferRing::getBufferRing(int pos)
{
    Buffer buf = init_Buffer();
    buf.state  = -1;

    if (pos<0) return buf;
    if (pos>=maxBufSize) pos = pos%maxBufSize;

    buf = make_Buffer(LBUF);
    copy_Buffer(&pBuf[pos], &buf);
    buf.state = inputSrc[pos];

    return buf;
}

