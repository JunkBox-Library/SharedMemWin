// MainFrm.h : CMainFrame クラスのインターフェイス
//

#pragma once

class  CNetProtocolApp;

class CMainFrame : public CFrameWnd
{
protected: // シリアル化からのみ作成します。
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

public:
    CNetProtocolApp*  pApp;

// オーバーライド
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
    virtual ~CMainFrame();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // コントロール バー用メンバ
    CStatusBar    m_wndStatusBar;
    CToolBar      m_wndToolBar;

// 生成された、メッセージ割り当て関数
protected:
    afx_msg void  OnUpdateUiNetStart(CCmdUI* pCmdUI);
    afx_msg void  OnUpdateUiNetStop(CCmdUI* pCmdUI);
    afx_msg void  OnUpdateUiNetSetting(CCmdUI* pCmdUI);
    afx_msg void  OnUpdateUiEditCopy(CCmdUI* pCmdUI);
    afx_msg void  OnUpdateUiLogSave(CCmdUI* pCmdUI);
    afx_msg void  OnUpdateUiLogClear(CCmdUI* pCmdUI);
    afx_msg int   OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void  OnInitMenu(CMenu* pMenu);
    DECLARE_MESSAGE_MAP()
    
};

