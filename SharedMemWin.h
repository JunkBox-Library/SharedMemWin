// SharedMemWin.h : SharedMemWin アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
    #error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル

#include "MainFrm.h"
#include "SharedMemWinDoc.h"
#include "SharedMemWinView.h"
#include "WinSMTool.h"

#include "SHM_thread.h"


class CSharedMemWinApp : public CWinApp
{
public:
    CSharedMemWinApp();
    ~CSharedMemWinApp();

public:
    CMainFrame*         pMainFrame;
    CSharedMemWinDoc*   pMainDoc;
    CSharedMemWinView*  pMainView;

    jbxwl::CWinSharedMem* pShm;

    ShmParam            m_shmparam;
    CWinThread*         p_shm_thr;

    int                 m_state;

public:

protected:

// オーバーライド
public:
    virtual  BOOL    InitInstance();

    afx_msg  void    OnLogSave();
    afx_msg  void    OnLogSaveAs();
    afx_msg  void    OnEditCopy();
    afx_msg  void    SHM_Start();
    afx_msg  void    SHM_Stop();
    afx_msg  void    SHM_Setting();
    afx_msg  void    OnLogClear();
    afx_msg  void    OnAppAbout();
    DECLARE_MESSAGE_MAP()
};


extern CSharedMemWinApp theApp;

