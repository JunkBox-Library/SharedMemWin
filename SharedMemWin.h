// NetProtocol.h : NetProtocol アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
    #error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル

#include "MainFrm.h"
#include "NetProtocolDoc.h"
#include "NetProtocolView.h"

#include "TCP_thread.h"


class CNetProtocolApp : public CWinApp
{
public:
    CNetProtocolApp();
    ~CNetProtocolApp();

public:
    CMainFrame*         pMainFrame;
    CNetProtocolDoc*    pMainDoc;
    CNetProtocolView*   pMainView;

    NetParam            m_netparam;
    CWinThread*         m_server_thr;

    int                 m_state;

public:

protected:

// オーバーライド
public:
    virtual  BOOL    InitInstance();

    afx_msg  void    OnLogSave();
    afx_msg  void    OnLogSaveAs();
    afx_msg  void    OnEditCopy();
    afx_msg  void    Server_Start();
    afx_msg  void    Server_Stop();
    afx_msg  void    Server_Setting();
    afx_msg  void    OnLogClear();
    afx_msg  void    OnAppAbout();
    DECLARE_MESSAGE_MAP()
};


extern CNetProtocolApp theApp;

