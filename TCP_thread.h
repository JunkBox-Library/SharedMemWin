
#ifndef  __TCP_THREAD_H__
#define  __TCP_THREAD_H__



#include "tools++.h"
#include "network.h"
#include "buffer.h"
#include "tlist.h"


#include "BufferRing.h"
#include "NetProtocolDoc.h"
#include "NetProtocolView.h"

#define   RELAY_NOSET   0    // 中継不能（未設定）
#define   RELAY_STOP    1    // 中継準備ＯＫ
#define   RELAY_EXEC    2    // 中継中


//extern  tList*  Thread_List;


#define  TIME_OUT       10


typedef struct {
    CString hostname;

    int     sport;
    int     cport;
    int     ssock;
    int     nsock;
    int     csock;

    BOOL    proxymode;
    BOOL    binhexmode;

    int*    p_state;

    HWND    hwnd;
    CNetProtocolDoc*  pDoc;
    CNetProtocolView* pView;
} NetParam;


UINT    ntpl_server(LPVOID pntprm);
UINT    ntpl_relay (LPVOID pntprm);
UINT    ntpl_web_proxy(LPVOID pntprm);

void    ntpl_tcp_relay(NetParam netparam);
int     ntpl_www2browser_relay(NetParam netparam, int btm, int wtm, int keep);

void    ntpl_saveBuffer(Buffer buf, NetParam netparam, int input);
void    ntpl_saveListHeader(tList* lp, NetParam netparam, int input);
void    ntpl_saveMessage(char* msg, NetParam netparam, int input);

void    ntpl_thread_stop(NetParam netparam);
tList*  find_thread(tList* list, int id);

#endif
