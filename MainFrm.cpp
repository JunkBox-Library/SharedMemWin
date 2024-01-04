// MainFrm.cpp : CMainFrame クラスの実装
//

#include "stdafx.h"
#include "SharedMemWin.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace jbxl;
//using namespace jbxwl;


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_UPDATE_COMMAND_UI(ID_SHM_START,   &CMainFrame::OnUpdateUiShmStart)
    ON_UPDATE_COMMAND_UI(ID_SHM_STOP,    &CMainFrame::OnUpdateUiShmStop)
    ON_UPDATE_COMMAND_UI(ID_SHM_SETTING, &CMainFrame::OnUpdateUiShmSetting)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY,   &CMainFrame::OnUpdateUiEditCopy)
    ON_UPDATE_COMMAND_UI(ID_LOG_SAVE,    &CMainFrame::OnUpdateUiLogSave)
    ON_UPDATE_COMMAND_UI(ID_LOG_CLEAR,   &CMainFrame::OnUpdateUiLogClear)

    ON_WM_CREATE()
    ON_WM_INITMENU()
END_MESSAGE_MAP()


/*
static UINT indicators[] =
{
    ID_SEPARATOR,           // ステータス ライン インジケータ
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};
*/


// CMainFrame コンストラクション/デストラクション

CMainFrame::CMainFrame()
{
    pApp = NULL;
    m_bAutoMenuEnable = FALSE;    // メニューの有効，無効の切り替えを可能にする
}


CMainFrame::~CMainFrame()
{
    //DEBUG_Error("ディストラクタ：IN  CMainFrame");
    //DEBUG_Error("ディストラクタ：OUT CMainFrame");
}



///////////////////////////////////////////////////////////////////////////////
// 初期化

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;
    
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("ツール バーの作成に失敗しました。\n");
        return -1;      // 作成できませんでした。
    }
    
    /*
    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("ステータス バーの作成に失敗しました。\n");
        return -1;      // 作成できませんでした。
    }
    */

    // TODO: ツール バーをドッキング可能にしない場合は、これらの 3 行を削除してください。
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    return 0;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) ) return FALSE;

    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// CMainFrame 診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}


void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG



///////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

// メニューの有効化と無効化
void  CMainFrame::OnInitMenu(CMenu* pMenu)
{
    if (pMenu==NULL) pMenu = GetMenu();

    if (pApp->m_state==SHM_NOSET) {        // 読込不能（未設定）
        pMenu->EnableMenuItem(ID_SHM_START,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        pMenu->EnableMenuItem(ID_SHM_STOP,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        pMenu->EnableMenuItem(ID_SHM_SETTING, MF_BYCOMMAND | MF_ENABLED);
        //m_wndToolBar.GetToolBarCtrl().EnableButton(ID_SHM_START, FALSE);
    }
    else if (pApp->m_state==SHM_STOP) {    // 読込準備ＯＫ
        pMenu->EnableMenuItem(ID_SHM_START,   MF_BYCOMMAND | MF_ENABLED);
        pMenu->EnableMenuItem(ID_SHM_STOP,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        pMenu->EnableMenuItem(ID_SHM_SETTING, MF_BYCOMMAND | MF_ENABLED);
    }
    else if (pApp->m_state==SHM_EXEC) {    // 読込中
        pMenu->EnableMenuItem(ID_SHM_START,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        pMenu->EnableMenuItem(ID_SHM_STOP,    MF_BYCOMMAND | MF_ENABLED);
        pMenu->EnableMenuItem(ID_SHM_SETTING, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED); 
    }

    CFrameWnd::OnInitMenu(pMenu);
}


void  CMainFrame::OnUpdateUiShmStart(CCmdUI* pCmdUI)
{
    if (pApp==NULL) return;

    if (pApp->m_state==SHM_NOSET) {
        pCmdUI->Enable(FALSE);
    }
    else if (pApp->m_state==SHM_STOP) {
        pCmdUI->Enable(TRUE);
    }
    else if (pApp->m_state==SHM_EXEC) {
        pCmdUI->Enable(FALSE);
    }
    //m_wndToolBar.GetToolBarCtrl().EnableButton(ID_SHM_START, FALSE);
}


void  CMainFrame::OnUpdateUiShmStop(CCmdUI* pCmdUI)
{
    if (pApp==NULL) return;

    if (pApp->m_state==SHM_NOSET) {
        pCmdUI->Enable(FALSE);
    }
    else if (pApp->m_state==SHM_STOP) {
        pCmdUI->Enable(FALSE);
    }
    else if (pApp->m_state==SHM_EXEC) {
        pCmdUI->Enable(TRUE);
    }
}


void  CMainFrame::OnUpdateUiShmSetting(CCmdUI* pCmdUI)
{
    if (pApp==NULL) return;

    if (pApp->m_state==SHM_NOSET) {
        pCmdUI->Enable(TRUE);
    }
    else if (pApp->m_state==SHM_STOP) {
        pCmdUI->Enable(TRUE);
    }
    else if (pApp->m_state==SHM_EXEC) {
        pCmdUI->Enable(FALSE);
    }
}


void  CMainFrame::OnUpdateUiEditCopy(CCmdUI* pCmdUI)
{
    if (pApp==NULL || pApp->pMainView==NULL) {
        pCmdUI->Enable(FALSE);
        return;
    }

    if (pApp->pMainView->m_copysy>=0) {
        pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }
}


void  CMainFrame::OnUpdateUiLogSave(CCmdUI* pCmdUI)
{
    if (pApp==NULL || pApp->pMainDoc==NULL || pApp->pMainDoc->bufferRing==NULL) {
        pCmdUI->Enable(FALSE);
        return;
    }

    if (pApp->pMainDoc->bufferRing->getMaxLineY()==0) {
        pCmdUI->Enable(FALSE);
    }
    else {
        pCmdUI->Enable(TRUE);
    }
}


void  CMainFrame::OnUpdateUiLogClear(CCmdUI* pCmdUI)
{
    if (pApp==NULL || pApp->pMainDoc==NULL || pApp->pMainDoc->bufferRing==NULL) {
        pCmdUI->Enable(FALSE);
        return;
    }

    if (pApp->pMainDoc->bufferRing->getMaxLineY()==0) {
        pCmdUI->Enable(FALSE);
    }
    else {
        pCmdUI->Enable(TRUE);
    }
}

