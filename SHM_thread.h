
#ifndef  __SHM_THREAD_H__
#define  __SHM_THREAD_H__


#include "tools++.h"
#include "network.h"
#include "buffer.h"
#include "tlist.h"

#include "BufferRing.h"
#include "SharedMemWinDoc.h"
#include "SharedMemWinView.h"
#include "WinSMTool.h"


#define   SHM_NOSET   0    // 読込不能（未設定）
#define   SHM_STOP    1    // 読込準備ＯＫ
#define   SHM_EXEC    2    // 読込中


typedef struct {
    BOOL    binhexmode;
    int*    p_state;

    HWND    hwnd;
    CSharedMemWinDoc*     pDoc;
    CSharedMemWinView*    pView;
    jbxwl::CWinSharedMem* pShm;
} ShmParam;


UINT    shm_reader(LPVOID pntprm);
void    shm_saveBuffer(Buffer* buf, ShmParam*p_shmparam, int input);

void    shm_thread_stop(ShmParam*p_shmparam);


#endif
