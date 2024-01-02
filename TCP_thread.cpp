

#include "stdafx.h"

#include "tools++.h"
#include "http_tool.h"

#include "TCP_thread.h"
#include "NetProtocol.h"


using namespace jbxl;
//using namespace jbxwl;


//FILE* fp;


UINT  ntpl_server(LPVOID pntprm)
{
    int    sofd, cdlen;
    struct sockaddr_in cl_addr;

    NetParam netparam = *(NetParam*)pntprm;

    while (*(netparam.p_state)==RELAY_EXEC) {
        cdlen = sizeof(cl_addr);
        sofd = accept(netparam.ssock, (struct sockaddr*)&cl_addr, (socklen_t*)&cdlen);
        if (sofd>0) {
            netparam.nsock = sofd;
            CWinThread* relay_thr = NULL;
            
            if (netparam.proxymode) {
                relay_thr = AfxBeginThread(ntpl_web_proxy, &netparam);
            }
            else {
                relay_thr = AfxBeginThread(ntpl_relay, &netparam);
            }
            //if (relay_thr!=NULL) {
            //    add_tList_node_bystr(Thread_List, (int)relay_thr->m_nThreadID, 0, NULL, NULL, (void*)relay_thr, sizeof(CWinThread*));
            //}
        }
    }
    ntpl_thread_stop(netparam);

    return 0;
}


UINT  ntpl_relay(LPVOID pntprm)
{
    NetParam netparam = *(NetParam*)pntprm;
    CNetProtocolDoc*  pDoc = netparam.pDoc;

    if (pDoc==NULL || pDoc->bufferRing==NULL) {
        DEBUG_ERROR("ntpl_relay: ERROR: nothing pDoc or pDoc->bufferRing");
        socket_close(netparam.nsock);
        return 1;
    }

    netparam.csock = tcp_client_socket((char*)(LPCSTR)(netparam.hostname), netparam.cport);
    if (netparam.csock<=0) {
        socket_close(netparam.nsock);  
        //Sleep(1000);
        //lp = find_thread(Thread_List, (int)GetCurrentThreadId());
        //if (lp!=NULL) del_tList_node(lp);
        return 1;
    }
    ntpl_tcp_relay(netparam);
    
    socket_close(netparam.csock);
    socket_close(netparam.nsock);

    //lp = find_thread(Thread_List, (int)GetCurrentThreadId());
    //if (lp!=NULL) del_tList_node(lp);
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
void  netp_web_proxy(int bofd, char* myip, int tmout)

    機能：簡易WEBプロキシ．WWW通信をリレーする．
          全てのHTTPプロトコルをサポートしている訳ではない．
          接続はできる限りの範囲でしか維持しない．

    引数：bofd -- ブラウザとの通信ソケット
          myip -- 自分のIPアドレス．
          tmout - タイムアウト

    注意：呼び出し側でソケット bofdをクローズすること．
          これを使用する特別の理由が無い限りは「ヤリイカ」などを使用してください．
*/
UINT  ntpl_web_proxy(LPVOID pntprm)
{
    int      len, cc, state;
    int      btm, wtm, keep;
    int      tmout;
    char*    ipa;
    tList*   pl;

    unsigned short sport = 0;
    Buffer   server, com;

    NetParam netparam = *(NetParam*)pntprm;


    //DEBUG_MODE print_message("[%d] SIMPLE_WEB_PROXY: Start Simple WEB Proxy.\n", (int)getpid());
    tmout = 30; // PROXY_TIMEOUT;
    btm = wtm = tmout;
    keep = TRUE;

    while (recv_wait(netparam.nsock, btm) && keep && *(netparam.p_state)==RELAY_EXEC) {
        cc = recv_http_header(netparam.nsock, &pl, &len, tmout, NULL, &state);
        if (cc<=0) break;
        if (pl==NULL) break;

        com = http_proxy_header_analyze(pl, &server, &sport, &btm, &keep);
        if (btm==0) btm = tmout;
        else        btm = Min(btm, 60);

        if (server.buf!=NULL && sport!=0) {
            ipa = get_ipaddr_byname_ipv4((char*)server.buf);
            char* myip = get_myipaddr_ipv4();
            if (ipa!=NULL && !strcmp(ipa, myip)) {
                free_Buffer(&server);
                server = make_Buffer_bystr("127.0.0.1");
            }
            freeNull(ipa);
            free(myip);

            netparam.csock = tcp_client_socket((char*)server.buf, sport);
        }
        if (server.buf!=NULL) free_Buffer(&server);

        if (netparam.csock<=0) {
            free_Buffer(&com);
            del_tList(&pl);
            break;
        }

        ntpl_saveListHeader(pl, netparam, CLIENT_DATA);

        // for HTTPS
        if (com.buf!=NULL && !strcasecmp((char*)com.buf, "CONNECT")) {
            Buffer sndbuf = make_Buffer_bystr("HTTP/1.1 200 Connection established.\r\nProxy-Connection: close\r\n\r\n");
            cc = tcp_send_Buffer(netparam.nsock, &sndbuf);
            ntpl_saveBuffer(sndbuf, netparam, PROXY_DATA);

            free_Buffer(&sndbuf);
            free_Buffer(&com);
            del_tList(&pl);

            ntpl_tcp_relay(netparam);
            break;
        }
        free_Buffer(&com);

        cc = send_http_header(netparam.csock, pl, ON);
        if (cc<len && state) {
            Buffer buf = make_Buffer(RECVBUFSZ);
            cc = recv_http_content(netparam.nsock, &buf, len-cc, wtm, NULL, &state);
            if (cc<=0) {
                del_tList(&pl);
                break;
            }

            tcp_send_Buffer(netparam.csock, &buf);
            ntpl_saveBuffer(buf, netparam, CLIENT_DATA);
            free_Buffer(&buf);
        }
        del_tList(&pl);

        cc = ntpl_www2browser_relay(netparam, btm, wtm, keep);
        if (cc<=0) break;
    }

    if (netparam.csock>0) socket_close(netparam.csock);
    if (netparam.nsock>0) socket_close(netparam.nsock);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////

void  ntpl_tcp_relay(NetParam netparam)
{
    int        cc, nm, nd;    
    fd_set  mask;
    Buffer  buf;
    struct timeval timeout;
    time_t otm, ntm;

    buf = make_Buffer(RECVBUFSZ);
    nm  = Max(netparam.nsock, netparam.csock);

    otm = time(NULL);
    do {
        timeout.tv_sec  = TIME_OUT;
        timeout.tv_usec = 0;
        FD_ZERO(&mask);
        FD_SET(netparam.csock, &mask);
        FD_SET(netparam.nsock, &mask);
        nd  = select(nm+1, &mask, NULL, NULL, &timeout);
        ntm = time(NULL);
    } while ((nd<0 || (!FD_ISSET(netparam.csock, &mask) && !FD_ISSET(netparam.nsock, &mask))) && (int)(ntm-otm)<=TIME_OUT);

    while((FD_ISSET(netparam.csock, &mask) || FD_ISSET(netparam.nsock, &mask)) && *(netparam.p_state)==RELAY_EXEC) {
        if (FD_ISSET(netparam.csock, &mask)) {
            cc = tcp_recv_Buffer(netparam.csock, &buf);
            if (cc>0) {
                //DEBUG_Error("%s", buf.buf);
                cc = tcp_send_Buffer(netparam.nsock, &buf);
                ntpl_saveBuffer(buf, netparam, SERVER_DATA);
                if (cc<=0) break;
            }
            else break;
        }

        if (FD_ISSET(netparam.nsock, &mask)) {
            cc = tcp_recv_Buffer(netparam.nsock, &buf);
            if (cc>0) {
                //DEBUG_Warning("%s", buf.buf);
                cc = tcp_send_Buffer(netparam.csock, &buf);
                ntpl_saveBuffer(buf, netparam, CLIENT_DATA);
                if (cc<=0) break;
            }
            else break;
        }

        otm = time(NULL);
        do {
            timeout.tv_sec  = TIME_OUT;
            timeout.tv_usec = 0;
            FD_ZERO(&mask);
            FD_SET(netparam.csock, &mask);
            FD_SET(netparam.nsock, &mask);
            nd  = select(nm+1, &mask, NULL, NULL, &timeout);
            ntm = time(NULL);
        } while ((nd<0 || (!FD_ISSET(netparam.csock, &mask) && !FD_ISSET(netparam.nsock, &mask))) && (int)(ntm-otm)<=TIME_OUT);
    }
    
    free_Buffer(&buf);

    return;
}


/**
int  ntpl_www2browser_relay(NetParam netparam, int btm, int wtm, int keep)

    機能: WWW -> Browser の通信をリレーする．

    引数：btm  -- ブラウザとのタイムアウト
          wtm  -- WWWサーバとのタイムアウト
          keep -- TRUE  接続を（可能な限り）維持する
                  FALSE 接続を維持しない．

    戻り値：<0    -- エラー
            NET_RECV_TIMEOUT  タイムアウト
*/
int  ntpl_www2browser_relay(NetParam netparam, int btm, int wtm, int keep)
{
    int    len, cc, cx, sz, tout, state;
    Buffer buf;
    tList* pl;
    tList* lst;

    if (!recv_wait(netparam.csock, wtm)) return JBXL_NET_RECV_TIMEOUT;
    cc = recv_http_header(netparam.csock, &pl, &len, wtm, NULL, &state);
    if (cc<=0) {
        if (pl!=NULL) del_tList(&pl);
        return cc;
    }
    if (pl==NULL) return -1;

    lst = strncasecmp_tList(pl, HDLIST_END_KEY, 0, 1);
    if (lst!=NULL && lst->prev!=NULL) {
        if (keep) {
            char   pxcn[48];
            snprintf(pxcn, 46, "keep-alive, timeout=%d", btm);
            add_tList_node_str(lst->prev, "Proxy-Connection", pxcn);
        }
        else {
            add_tList_node_str(lst->prev, "Proxy-Connection", "close");
        }
    }

    sz = cx = send_http_header(netparam.nsock, pl, ON);
    ntpl_saveListHeader(pl, netparam, SERVER_DATA);
    del_tList(&pl);

    if (len>0) {
        buf = make_Buffer(RECVBUFSZ);
        while(sz<len) {
            if (!recv_wait(netparam.csock, wtm)) {
                free_Buffer(&buf);
                return JBXL_NET_RECV_TIMEOUT;
            }
            cc = tcp_recv_Buffer(netparam.csock, &buf);
            if (cc>0) {
                sz += cc;
                cx = tcp_send_Buffer(netparam.nsock, &buf);
                ntpl_saveBuffer(buf, netparam, SERVER_DATA);
                if (cx<=0) break;
            }
            else break;
            //memset(buf.buf, 0, cc);
        }

        free_Buffer(&buf);
        if (cc<=0 || (cc>0&&cx<=0)) return 0;
    }

    else if (len==HTTP_HEADER_CHUNKED) {
        buf = make_Buffer(RECVBUFSZ);
        while ((tout=recv_wait(netparam.csock, 5))){
            cc = tcp_recv_Buffer(netparam.csock, &buf);
            if (cc>0) {
                sz += cc;
                cx = tcp_send_Buffer(netparam.nsock, &buf);
                ntpl_saveBuffer(buf, netparam, SERVER_DATA);
                if (cx<=0) break;
            }
            else break;
            //memset(buf.buf, 0, cc);
        }

        free_Buffer(&buf);
        if (!tout) return JBXL_NET_RECV_TIMEOUT;

        //if (len==HTTP_HEADER_CHUNKED) return 0;        // chunked モードではその都度切断．
        //if (cc<=0 || (cc>0&&cx<=0)) return 0;
        return 0;
    }

    return sz;
}



//////////////////////////////////////////////////////////////////////////////////////////////

void  ntpl_saveBuffer(Buffer buf, NetParam netparam, int input)
{
    CBufferRing* pBR = netparam.pDoc->bufferRing;
    CSingleLock lock(&(netparam.pDoc->criticalKey));

    lock.Lock();
    while (!lock.IsLocked()) {
        DEBUG_WARN("ntpl_saveBuffer: WARNING: lock is failed.");
        Sleep(100);
        lock.Lock();
    }
    pBR->putBufferRing(buf, input, BINHEX_DATA);
    
    int lastPos = pBR->getLastPosition();
    //if (netparam.binhexmode && pBR->getKindData(lastPos - 1) == BINARY_DATA) {
    if (netparam.binhexmode) {
        pBR->rewriteBinHexBufferRing(lastPos-1, input);
    }
    lock.Unlock();

    InvalidateRect(netparam.hwnd, NULL, FALSE);

    return;
}


void  ntpl_saveListHeader(tList* lp, NetParam netparam, int input)
{
    CBufferRing* pBR = netparam.pDoc->bufferRing;
    CSingleLock lock(&(netparam.pDoc->criticalKey));

    lock.Lock();
    while (!lock.IsLocked()) {
        DEBUG_WARN("ntpl_saveListHeader: WARNING: lock is failed.");
        Sleep(100);
        lock.Lock();
    }

    Buffer buf = restore_protocol_header(lp, ": ", ON, NULL);
    pBR->putBufferRing(buf, input, BINHEX_DATA);
    
    int lastPos = pBR->getLastPosition();
    //if (netparam.binhexmode && pBR->getKindData(lastPos - 1) == BINARY_DATA) {
    if (netparam.binhexmode) {
        pBR->rewriteBinHexBufferRing(lastPos-1, input);
    }

    lock.Unlock();

    InvalidateRect(netparam.hwnd, NULL, FALSE);
    free_Buffer(&buf);

    return;
}


void  ntpl_saveMessage(char* msg, NetParam netparam, int input)
{
    CBufferRing* pBR = netparam.pDoc->bufferRing;
    CSingleLock lock(&(netparam.pDoc->criticalKey));

    Buffer buf = make_Buffer_bystr(msg);

    lock.Lock();
    while (!lock.IsLocked()) {
        DEBUG_WARN("ntpl_saveMessage: WARNING: lock is failed.");
        Sleep(100);
        lock.Lock();
    }
    pBR->putBufferRing(buf, input, BINHEX_DATA);

    int lastPos = pBR->getLastPosition();
    //if (netparam.binhexmode && pBR->getKindData(lastPos - 1) == BINARY_DATA) {
    if (netparam.binhexmode) {
        pBR->rewriteBinHexBufferRing(lastPos - 1, input);
    }
    lock.Unlock();

    InvalidateRect(netparam.hwnd, NULL, FALSE);
    free_Buffer(&buf);

    return;
}


void  ntpl_thread_stop(NetParam netparam)
{
    *(netparam.p_state) = RELAY_STOP;

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


tList*  find_thread(tList* list, int id)
{
    tList* lp = list;

    while (lp!=NULL) {
        if (lp->ldat.id==id) return lp;
        lp = lp->next;
    }
    return NULL;
}

