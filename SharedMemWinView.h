// NetProtocolView.h : CNetProtocolView クラスのインターフェイス
//

#pragma once

#include "afxwin.h"

#define  DISPLAY_MARGIN  5

class  CNetProtocolDoc;
class  CNetProtocolApp;

class CNetProtocolView : public CScrollView
{
protected: // シリアル化からのみ作成します。
    CNetProtocolView();
    DECLARE_DYNCREATE(CNetProtocolView)

public:
    CNetProtocolDoc* GetDocument() const;
    CNetProtocolApp* pApp;

// オーバーライド
public:
    virtual  BOOL    PreCreateWindow(CREATESTRUCT& cs);

protected:
    virtual  void    OnInitialUpdate(); // 構築後に初めて呼び出されます。
    virtual  BOOL    OnPreparePrinting(CPrintInfo* pInfo);
    virtual  void    OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual  void    OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:
    virtual ~CNetProtocolView();

#ifdef _DEBUG
    virtual  void    AssertValid() const;
    virtual  void    Dump(CDumpContext& dc) const;
#endif


// 生成された、メッセージ割り当て関数
protected:
    int       m_msx;
    int       m_msy;
    int       m_mex;
    int       m_mey;

    int       m_mpress;
    int       m_dragsx;
    int       m_dragsy;
    int       m_dragex;
    int       m_dragey;

    afx_msg  int    OnCreate(LPCREATESTRUCT lpcs);
    afx_msg  void   OnDestroy(void);
    afx_msg  void   OnDraw(CDC* pDC);
    DECLARE_MESSAGE_MAP()

public:
    CFont     m_ScreenFont;
    int       m_FontSizeX;
    int       m_FontSizeY;

    int       m_ScrollSizeX;
    int       m_ScrollSizeY;
    int       m_DocLastPos;

    int       m_copysy;
    int       m_copyey;

    void      resetScrollSize(void);
    void      setScrollPosition(CDC* pDC, CBufferRing* pBR);
    void      clearViewDoc(void);
    CString   getCopyData(void);
    BOOL      OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll);

    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


#ifndef _DEBUG  // NetProtocolView.cpp のデバッグ バージョン
inline CNetProtocolDoc* CNetProtocolView::GetDocument() const
   { return reinterpret_cast<CNetProtocolDoc*>(m_pDocument); }
#endif

