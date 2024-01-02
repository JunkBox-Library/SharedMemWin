// NetProtocol.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "NetProtocol.h"

#include "TCP_thread.h"
#include "NetSettingDLG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace jbxl;
//using namespace jbxwl;

//tList*   Thread_List = NULL;


// Command
BEGIN_MESSAGE_MAP(CNetProtocolApp, CWinApp)
    ON_COMMAND(ID_EDIT_COPY,    &CNetProtocolApp::OnEditCopy)
    ON_COMMAND(ID_NET_START,    &CNetProtocolApp::Server_Start)
    ON_COMMAND(ID_NET_STOP,     &CNetProtocolApp::Server_Stop)
    ON_COMMAND(ID_NET_SETTING,  &CNetProtocolApp::Server_Setting)
    ON_COMMAND(ID_LOG_SAVE,     &CNetProtocolApp::OnLogSave)
    ON_COMMAND(ID_LOG_SAVE_AS,  &CNetProtocolApp::OnLogSaveAs)
    ON_COMMAND(ID_LOG_CLEAR,    &CNetProtocolApp::OnLogClear)
    ON_COMMAND(ID_APP_ABOUT,    &CNetProtocolApp::OnAppAbout)
    // 標準のファイル基本ドキュメント コマンド
    //ON_COMMAND(ID_FILE_NEW,  &CWinApp::OnFileNew)
    //ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
    // 標準の印刷セットアップ コマンド
    //ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


CNetProtocolApp::CNetProtocolApp()
{    
    pMainFrame   = NULL;
    pMainDoc     = NULL;
    m_server_thr = NULL;
    m_state      = RELAY_NOSET;

    //m_netparam.hostname = "proxy.edu.tuis.ac.jp";
    //m_netparam.cport    = 8080;
    m_netparam.hostname   = "";
    m_netparam.cport      = 0;
    
    m_netparam.sport      = 9100;
    m_netparam.ssock      = 0;
    m_netparam.nsock      = 0;
    m_netparam.csock      = 0;

    m_netparam.proxymode  = FALSE;
    m_netparam.binhexmode = FALSE;

    m_netparam.p_state    = &m_state;
    m_netparam.hwnd       = NULL;
    m_netparam.pDoc       = NULL;
    m_netparam.pView      = NULL;
}


CNetProtocolApp::~CNetProtocolApp()
{
    //DEBUG_Error("ディストラクタ：IN  CNetProtocolApp");

    // for Network
    //if (m_netparam.ssock!=0) {
    //    socket_close(m_netparam.ssock);
    //    m_netparam.ssock = 0;
    //}
    Server_Stop();
    cleanup_network();

    //DEBUG_Error("ディストラクタ：OUT CNetProtocolApp");
}


// 唯一の CNetProtocolApp オブジェクトです。
CNetProtocolApp theApp;


// CNetProtocolApp 初期化
BOOL CNetProtocolApp::InitInstance()
{
    CWinApp::InitInstance();

    SetRegistryKey(_T("東京情報大学 NSL ネットワークアプリケーション"));
    LoadStdProfileSettings(4);            // 標準の INI ファイルのオプションをロードします (MRU を含む)

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CNetProtocolDoc),
        RUNTIME_CLASS(CMainFrame),       // メイン SDI フレーム ウィンドウ
        RUNTIME_CLASS(CNetProtocolView));
    if (!pDocTemplate) return FALSE;
    AddDocTemplate(pDocTemplate);

    // DDE、file open など標準のシェル コマンドのコマンド ラインを解析します。
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    // コマンド ラインで指定されたディスパッチ コマンドです。アプリケーションが
    // /RegServer、/Register、/Unregserver または /Unregister で起動された場合、False を返します。
    if (!ProcessShellCommand(cmdInfo)) return FALSE;

    // メイン ウィンドウが初期化されたので、表示と更新を行います。
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();
    // 接尾辞が存在する場合にのみ DragAcceptFiles を呼び出してください。
    //  SDI アプリケーションでは、ProcessShellCommand の直後にこの呼び出しが発生しなければなりません。

    // Frame
    pMainFrame = (CMainFrame*)m_pMainWnd;
    pMainDoc   = (CNetProtocolDoc*)pMainFrame->GetActiveDocument();
    pMainView  = ((CNetProtocolDoc*)pMainDoc)->GetView();

    pMainFrame->pApp = this;
    pMainDoc->pApp   = this;
    pMainView->pApp  = this;

    // ネットワーク
    init_network();

    return TRUE;
}


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// ダイアログ データ
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
    DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


void  CNetProtocolApp::OnLogSave()
{
    if (pMainDoc->save_fname=="") {
        pMainDoc->save_fname = pMainDoc->easyGetSaveFileName("保存用ファイルを指定する", m_pMainWnd->m_hWnd);
    }
    if (pMainDoc->save_fname=="") return;
    int ret = pMainDoc->writeLogFile();

    if (ret<0) MessageBox(m_pMainWnd->m_hWnd, "ファイルの書き込みに失敗しました", "エラー", MB_OK);
}


void  CNetProtocolApp::OnLogSaveAs()
{
    pMainDoc->save_fname = "";

    OnLogSave();
}


void  CNetProtocolApp::OnEditCopy()
{
    CString data = pMainView->getCopyData();
    if (data=="") return;

    HGLOBAL hMem = ::GlobalAlloc(GHND, data.GetLength());
    if (hMem == NULL) return;
    char* pszptr = (char*)::GlobalLock(hMem);
    lstrcpy(pszptr, data);
    ::GlobalUnlock(hMem);
 
    if (!OpenClipboard(NULL)) {
        ::GlobalFree(hMem);
        return;
    }
    if (!EmptyClipboard()) {
        ::GlobalFree(hMem);
        return;
    }
    if (SetClipboardData(CF_TEXT, hMem) == NULL) {
        ::GlobalFree(hMem);
    }

    CloseClipboard();
    return;
}


// ダイアログを実行するためのアプリケーション コマンド
void  CNetProtocolApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


// CNetProtocolApp メッセージ ハンドラ

void  CNetProtocolApp::Server_Start()
{
    if (m_state!=RELAY_STOP) return;

    if (m_netparam.sport<=0) {
        MessageBox(m_pMainWnd->m_hWnd, "Server_Setart: 不正なサーバポート番号です", "エラー", MB_OK);
    }
    else {
        m_netparam.ssock = tcp_server_socket_ipv4(m_netparam.sport);
        if (m_netparam.ssock<=0) {
            m_netparam.ssock = 0;
            m_state = RELAY_STOP;
            MessageBox(m_pMainWnd->m_hWnd, "ntpl_server: サーバポートのオープンに失敗", "エラー", MB_OK);
            return;
        }
        m_state = RELAY_EXEC;
        m_server_thr = AfxBeginThread(ntpl_server, &m_netparam);
        if (m_server_thr==NULL) m_state = RELAY_STOP;
    }
    return;
}


void  CNetProtocolApp::Server_Stop()
{
    m_state = RELAY_STOP;
    if (m_netparam.ssock>0) {
        socket_close(m_netparam.ssock);
        m_netparam.ssock = 0;
    }
}


void  CNetProtocolApp::Server_Setting()
{
    NetSettingDLG* nstdlg = new NetSettingDLG(m_netparam);
    if (nstdlg==NULL) return;
    
    if (nstdlg->DoModal()==IDOK) {
        m_netparam = nstdlg->GetParameter();
        m_netparam.pDoc  = pMainDoc;
        m_netparam.pView = pMainView;
        m_netparam.hwnd  = m_pMainWnd->m_hWnd;
        m_state = RELAY_NOSET;

        if (!m_netparam.proxymode) {
            if (m_netparam.sport<=0 || m_netparam.hostname=="") {
                MessageBox(m_pMainWnd->m_hWnd, "Server_Setting: 不正なリモートホストが指定されました", "エラー", MB_OK);
            }
            else {
                int cofd = tcp_client_socket((char*)(LPCSTR)(m_netparam.hostname), m_netparam.cport);
                if (cofd>0) {
                    socket_close(cofd);
                    m_state = RELAY_STOP;
                }
                else {
                    MessageBox(m_pMainWnd->m_hWnd, "Server_Setting: リモートホストに接続できません", "エラー", MB_OK);
                }
            }
        }
        else {
            m_state = RELAY_STOP;
        }
    }

    delete(nstdlg);
    return;
}


void  CNetProtocolApp::OnLogClear()
{
    int ret = MessageBox(m_pMainWnd->m_hWnd, "ログをクリアしますか？", "Log Clear", MB_YESNO | MB_ICONQUESTION);
    if (ret==IDYES) pMainView->clearViewDoc();
}
