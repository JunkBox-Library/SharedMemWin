// SharedMemWin.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "SharedMemWin.h"

#include "SHM_thread.h"
#include "MemSettingDLG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace jbxl;
//using namespace jbxwl;

//tList*   Thread_List = NULL;


// Command
BEGIN_MESSAGE_MAP(CSharedMemWinApp, CWinApp)
    ON_COMMAND(ID_EDIT_COPY,    &CSharedMemWinApp::OnEditCopy)
    ON_COMMAND(ID_SHM_START,    &CSharedMemWinApp::SHM_Start)
    ON_COMMAND(ID_SHM_STOP,     &CSharedMemWinApp::SHM_Stop)
    ON_COMMAND(ID_SHM_SETTING,  &CSharedMemWinApp::SHM_Setting)
    ON_COMMAND(ID_LOG_SAVE,     &CSharedMemWinApp::OnLogSave)
    ON_COMMAND(ID_LOG_SAVE_AS,  &CSharedMemWinApp::OnLogSaveAs)
    ON_COMMAND(ID_LOG_CLEAR,    &CSharedMemWinApp::OnLogClear)
    ON_COMMAND(ID_APP_ABOUT,    &CSharedMemWinApp::OnAppAbout)
    // 標準のファイル基本ドキュメント コマンド
    //ON_COMMAND(ID_FILE_NEW,  &CWinApp::OnFileNew)
    //ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
    // 標準の印刷セットアップ コマンド
    //ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


CSharedMemWinApp::CSharedMemWinApp()
{    
    pMainFrame = NULL;
    pMainDoc   = NULL;
    pShm       = NULL;
    p_shm_thr  = NULL;
    m_state    = SHM_NOSET;

    m_shmparam.p_state = &m_state;
    m_shmparam.hwnd    = NULL;
    m_shmparam.pDoc    = NULL;
    m_shmparam.pView   = NULL;
    m_shmparam.pShm    = NULL;
}


CSharedMemWinApp::~CSharedMemWinApp()
{
    //DEBUG_Error("ディストラクタ：IN  CSharedMemWinApp");

    // for Network
    //if (m_p_shmparam->ssock!=0) {
    //    socket_close(m_p_shmparam->ssock);
    //    m_p_shmparam->ssock = 0;
    //}
    SHM_Stop();
    cleanup_network();

    //DEBUG_Error("ディストラクタ：OUT CSharedMemWinApp");
}


// 唯一の CSharedMemWinApp オブジェクトです。
CSharedMemWinApp theApp;


// CSharedMemWinApp 初期化
BOOL CSharedMemWinApp::InitInstance()
{
    CWinApp::InitInstance();

    SetRegistryKey(_T("東京情報大学 NSL ネットワークアプリケーション"));
    LoadStdProfileSettings(4);            // 標準の INI ファイルのオプションをロードします (MRU を含む)

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CSharedMemWinDoc),
        RUNTIME_CLASS(CMainFrame),       // メイン SDI フレーム ウィンドウ
        RUNTIME_CLASS(CSharedMemWinView));
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
    pMainDoc   = (CSharedMemWinDoc*)pMainFrame->GetActiveDocument();
    pMainView  = ((CSharedMemWinDoc*)pMainDoc)->GetView();

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


void  CSharedMemWinApp::OnLogSave()
{
    if (pMainDoc->save_fname=="") {
        pMainDoc->save_fname = pMainDoc->easyGetSaveFileName("保存用ファイルを指定する", m_pMainWnd->m_hWnd);
    }
    if (pMainDoc->save_fname=="") return;
    int ret = pMainDoc->writeLogFile();

    if (ret<0) MessageBox(m_pMainWnd->m_hWnd, _T("ファイルの書き込みに失敗しました"), _T("エラー"), MB_OK);
}


void  CSharedMemWinApp::OnLogSaveAs()
{
    pMainDoc->save_fname = "";

    OnLogSave();
}


void  CSharedMemWinApp::OnEditCopy()
{
    CString data = pMainView->getCopyData();
    if (data=="") return;

    HGLOBAL hMem = ::GlobalAlloc(GHND, data.GetLength());
    if (hMem == NULL) return;
    char* pszptr = (char*)::GlobalLock(hMem);
    //lstrcpy(pszptr, data);
    memcpy(pszptr, data, data.GetLength());

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
void  CSharedMemWinApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


// CSharedMemWinApp メッセージ ハンドラ

void  CSharedMemWinApp::SHM_Start()
{
    if (m_state!=SHM_STOP) return;

    pShm = new jbxwl::CWinSharedMem("jbxwl_DEBUGGER");
    if (pShm = NULL) {
        MessageBox(m_pMainWnd->m_hWnd, _T("SHM_Setart: 共有メモリをオープンできません"), _T("エラー"), MB_OK);
    }
    else {
        m_state = SHM_EXEC;
        m_shmparam.pShm = pShm;
        p_shm_thr = AfxBeginThread(shm_reader, &m_shmparam);
        if (p_shm_thr==NULL) m_state = SHM_STOP;
    }
    return;
}


void  CSharedMemWinApp::SHM_Stop()
{
    m_state = SHM_STOP;

    if (pShm!=NULL) {
        delete(pShm);
        pShm = NULL;
        m_shmparam.pShm = NULL;
    }
}


void  CSharedMemWinApp::SHM_Setting()
{
    MemSettingDLG* shmdlg = new MemSettingDLG(m_shmparam);
    if (shmdlg==NULL) return;
    
    if (shmdlg->DoModal()==IDOK) {
        m_shmparam = shmdlg->GetParameter();
        m_shmparam.pDoc  = pMainDoc;
        m_shmparam.pView = pMainView;
        m_shmparam.hwnd  = m_pMainWnd->m_hWnd;
        m_state = SHM_NOSET;
    }

    delete(shmdlg);
    return;
}


void  CSharedMemWinApp::OnLogClear()
{
    int ret = MessageBox(m_pMainWnd->m_hWnd, _T("ログをクリアしますか？"), _T("Log Clear"), MB_YESNO | MB_ICONQUESTION);
    if (ret==IDYES) pMainView->clearViewDoc();
}
