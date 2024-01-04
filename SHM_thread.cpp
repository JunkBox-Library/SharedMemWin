

#include "stdafx.h"

#include "tools++.h"
#include "http_tool.h"

#include "SHM_thread.h"
#include "SharedMemWin.h"


using namespace jbxl;
//using namespace jbxwl;


UINT  shm_reader(LPVOID pntprm)
{
    ShmParam* p_shmparam = (ShmParam*)pntprm;

    while (*(p_shmparam->p_state)==SHM_EXEC) {
        int cc = p_shmparam->pShm->get();
        shm_saveBuffer(p_shmparam->pShm->buf, p_shmparam, INFO_DATA);
    }
    shm_thread_stop(p_shmparam);

    return 0;
}


///////////////////////////////////////

void  shm_saveBuffer(Buffer* buf, ShmParam* p_shmparam, int input)
{
    CBufferRing* pBR =p_shmparam->pDoc->bufferRing;
    CSingleLock lock(&(p_shmparam->pDoc->criticalKey));

    lock.Lock();
    while (!lock.IsLocked()) {
        Sleep(100);
        lock.Lock();
    }
    pBR->putBufferRing(buf, input, BINHEX_DATA);
    
    int lastPos = pBR->getLastPosition();
    if (p_shmparam->binhexmode) {
        pBR->rewriteBinHexBufferRing(lastPos-1, input);
    }
    lock.Unlock();

    InvalidateRect(p_shmparam->hwnd, NULL, FALSE);

    return;
}


void  shm_thread_stop(ShmParam* p_shmparam)
{
    *(p_shmparam->p_state) = SHM_STOP;

    /*
    tList* lp = Thread_List->next;
    while (lp!=NULL) {
        CWinThread* thr = (CWinThread*)(lp->ldat.ptr);
        if (thr!=NULL) thr->PostThreadMessageA(WM_QUIT, 0, 0);
        lp = lp->next;
    }
    del_tList(&(Thread_List->next));
    */
}
