// NetProtocolView.cpp : CNetProtocolView クラスの実装
//

#include "stdafx.h"
#include "NetProtocol.h"

#include "NetProtocolDoc.h"
#include "NetProtocolView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace jbxl;
//using namespace jbxwl;


IMPLEMENT_DYNCREATE(CNetProtocolView, CScrollView)

BEGIN_MESSAGE_MAP(CNetProtocolView, CScrollView)
    ON_WM_CREATE() 
    ON_WM_DESTROY() 
    // 標準印刷コマンド
    //ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
    //ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
    //ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


CNetProtocolView::CNetProtocolView()
{
    m_FontSizeX   = 0;
    m_FontSizeY   = 0;
    m_DocLastPos  = 0;

    m_msx = m_msy = 0;
    m_mex = m_mey = 0;
    m_mpress      = FALSE;
    m_dragsx      = 0;
    m_dragsy      = 0;
    m_dragex      = 0;
    m_dragey      = 0;

    m_copysy      = -1;
    m_copyey      = -1;

    KanjiCode = CODE_SJIS;
    pApp = NULL;
}


CNetProtocolView::~CNetProtocolView()
{    
    //DEBUG_Error("ディストラクタ：IN  CNetProtocolView");

    pApp->m_state = RELAY_STOP;

    //DEBUG_Error("ディストラクタ：OUT CNetProtocolView");
}


BOOL CNetProtocolView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CScrollView::PreCreateWindow(cs);
}


void CNetProtocolView::OnDraw(CDC* pDC)
{
    CNetProtocolDoc* pDoc = GetDocument();
    if (!pDoc||pDoc->bufferRing==NULL) return;

    CBufferRing* pBR = pDoc->bufferRing;
    if (pBR->getMaxLineY()==0) return;

    CSingleLock lock(&(pDoc->criticalKey));
    lock.Lock();
    while (!lock.IsLocked()) {
        Sleep(100);
        lock.Lock();
    }

    resetScrollSize();

    CFont* pOldFontX = pDC->SelectObject(&m_ScreenFont);
    CRect  rc;
    pDC->GetClipBox(&rc);

    int nLineBegin = rc.top/m_FontSizeY;
    int nLineEnd   = min((rc.bottom+m_FontSizeY-1)/m_FontSizeY, pBR->getMaxLineY());

    for (int i=nLineBegin; i<nLineEnd; i++) {
        Buffer buf = pBR->getBufferRing(i);
        if (buf.buf==NULL) break;

        if (i>=m_copysy && i<=m_copyey) {
            pDC->SetBkColor(RGB(200,200,200));
        }
        else {
            pDC->SetBkColor(RGB(255,255,255));
        }

        if (buf.state==CLIENT_DATA) {
            pDC->SetTextColor(RGB(20,20,200));
        }
        else if (buf.state==SERVER_DATA) {
            pDC->SetTextColor(RGB(200,20,20));
        }
        else {
            pDC->SetTextColor(RGB(0,0,0));
        }

        //if (isText_Buffer(buf)) kanji_convert_Buffer(&buf);
        pDC->TextOut(DISPLAY_MARGIN, i*m_FontSizeY+DISPLAY_MARGIN, (LPCTSTR)buf.buf);
        free_Buffer(&buf);
    }

    setScrollPosition(pDC, pBR);

    lock.Unlock();

    pDC->SelectObject(pOldFontX);
}


void  CNetProtocolView::setScrollPosition(CDC* pDC, CBufferRing* pBR)
{
    int lastPos = pBR->getLastPosition();
    if (m_DocLastPos==lastPos) return;
    m_DocLastPos = lastPos;

    CRect  rc;
    GetClientRect(&rc);        

    POINT  pt;
    pt.x = 0;
    pt.y = (lastPos+1)*m_FontSizeY - rc.bottom + rc.top;
    if (pt.y<0) pt.y = 0;

    ScrollToPosition(pt);
}


void  CNetProtocolView::resetScrollSize()
{
    CNetProtocolDoc* pdoc = GetDocument();
    if (!pdoc || !pdoc->bufferRing) return;
    CBufferRing* pBR = pdoc->bufferRing;
    if (pBR->getMaxLineY()==0 || pBR->getMaxLineX()==0) return;
    
    int scrollSizeX = m_FontSizeX*pBR->getMaxLineX() + DISPLAY_MARGIN;
    int scrollSizeY = m_FontSizeY*pBR->getMaxLineY() + DISPLAY_MARGIN;
    //if (scrollSizeY>SHRT_MAX) scrollSizeY = SHRT_MAX;

    if (scrollSizeX!=m_ScrollSizeX || scrollSizeY!=m_ScrollSizeY) {
        m_ScrollSizeX = scrollSizeX;
        m_ScrollSizeY = scrollSizeY;

        CSize sizeTotal(0, 0);
        sizeTotal.cx = m_ScrollSizeX;
        sizeTotal.cy = m_ScrollSizeY;
        SetScrollSizes(MM_TEXT, sizeTotal);
    }

    return;
}


void  CNetProtocolView::OnInitialUpdate()
{
    CScrollView::OnInitialUpdate();

    CNetProtocolDoc* pDoc = GetDocument();
    if (!pDoc || pDoc->bufferRing==NULL) return;
    CBufferRing* pBR = pDoc->bufferRing;

    CClientDC dc(this);
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);

    m_FontSizeX   = dc.GetTextExtent(CString('0', 1)).cx;        // フォントの横サイズ
    m_FontSizeY   = tm.tmHeight + tm.tmExternalLeading;            // フォント（含行間）の縦サイズ

    m_ScrollSizeX = m_FontSizeX + DISPLAY_MARGIN;
    m_ScrollSizeY = m_FontSizeY + DISPLAY_MARGIN;
    //if (m_ScrollSizeY>SHRT_MAX) m_ScrollSizeY = SHRT_MAX;

    CSize sizeTotal(0, 0);
    sizeTotal.cx = m_ScrollSizeX + 2;
    sizeTotal.cy = m_ScrollSizeY;
    SetScrollSizes(MM_TEXT, sizeTotal);

    return;
}


void  CNetProtocolView::clearViewDoc()
{
    CNetProtocolDoc* pDoc = GetDocument();
    if (!pDoc || pDoc->bufferRing==NULL) return;
    CBufferRing* pBR = pDoc->bufferRing;


    CSingleLock lock(&(pDoc->criticalKey));
    lock.Lock();
    while (!lock.IsLocked()) {
        Sleep(100);
        lock.Lock();
    }

    m_ScrollSizeX = m_FontSizeX + DISPLAY_MARGIN;
    m_ScrollSizeY = m_FontSizeY + DISPLAY_MARGIN;
    //if (m_ScrollSizeY>SHRT_MAX) m_ScrollSizeY = SHRT_MAX;

    CSize sizeTotal(0, 0);
    sizeTotal.cx = m_ScrollSizeX + 2;
    sizeTotal.cy = m_ScrollSizeY;
    SetScrollSizes(MM_TEXT, sizeTotal);

    POINT  pt;
    pt.x = 0;
    pt.y = 0;
    ScrollToPosition(pt);

    pDoc->clear();

    InvalidateRect(NULL, FALSE);

    lock.Unlock();
}



///////////////////////////////////////////////////////////////////////////////
// CNetProtocolView 印刷

BOOL CNetProtocolView::OnPreparePrinting(CPrintInfo* pInfo)
{
    return DoPreparePrinting(pInfo);
}


void CNetProtocolView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: 印刷前の特別な初期化処理を追加してください。
}



void CNetProtocolView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: 印刷後の後処理を追加してください。
}



///////////////////////////////////////////////////////////////////////////////
// CNetProtocolView 診断

#ifdef _DEBUG
void CNetProtocolView::AssertValid() const
{
    CScrollView::AssertValid();
}


void CNetProtocolView::Dump(CDumpContext& dc) const
{
    CScrollView::Dump(dc);
}


CNetProtocolDoc* CNetProtocolView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNetProtocolDoc)));
    return (CNetProtocolDoc*)m_pDocument;
}
#endif //_DEBUG



///////////////////////////////////////////////////////////////////////////////
// CNetProtocolView メッセージ ハンドラ
// フォントの設定

int CNetProtocolView::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CScrollView::OnCreate(lpcs)==-1) return -1;

    CClientDC dc(this);
    m_ScreenFont.CreatePointFont(120, _T("ＭＳ ゴシック"), &dc);

    return 0;
}


void CNetProtocolView::OnDestroy(void)
{
    m_ScreenFont.DeleteObject();
    CScrollView::OnDestroy();
}



////////////////////////////////////////////////////////////////////////////////////////////
// Copy (& Paste)
// 
void  CNetProtocolView::OnLButtonDown(UINT nFlags, CPoint point)
{
    CPoint scrlpos = GetScrollPosition();
    m_msx     = point.x + scrlpos.x;
    m_msy     = point.y + scrlpos.y;
    m_dragsx = m_msx/m_FontSizeX;
    m_dragsy = m_msy/m_FontSizeY;
    m_mpress = TRUE;

    // Window内をクリック
    CRect  rc;
    GetClientRect(&rc);        
    if (point.x>=rc.left && point.x<=rc.right && point.y>rc.top && point.y<=rc.bottom) {
        m_copysy = -1;
        m_copyey = -1;
        InvalidateRect(NULL, FALSE);
    }

    CScrollView::OnLButtonDown(nFlags, point);
}


void  CNetProtocolView::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_mpress = FALSE;

    CScrollView::OnLButtonUp(nFlags, point);
}


void  CNetProtocolView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_mpress) {
        CPoint scrlpos = GetScrollPosition();
        m_mex = point.x + scrlpos.x;
        m_mey = point.y + scrlpos.y;
        m_dragex = m_mex/m_FontSizeX;
        m_dragey = m_mey/m_FontSizeY;

        if (m_dragsx!=m_dragex || m_dragsy!=m_dragey) {
            m_copysy = m_dragsy;
            m_copyey = m_dragey;
            InvalidateRect(NULL, FALSE);
        }
    }

    CScrollView::OnMouseMove(nFlags, point);
}


CString  CNetProtocolView::getCopyData(void)
{
    CString ret = "";

    CNetProtocolDoc* pdoc = GetDocument();
    if (!pdoc || !pdoc->bufferRing) return ret;
    CBufferRing* pBR = pdoc->bufferRing;

    for (int i=m_copysy; i<=m_copyey; i++) {
        ret = ret + (LPCTSTR)(pBR->pBuf[i].buf);
    }

    return ret;
}


BOOL  CNetProtocolView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
    SCROLLINFO info;
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask  = SIF_TRACKPOS;

    if (LOBYTE(nScrollCode)==SB_THUMBTRACK)
    {
        GetScrollInfo(SB_HORZ, &info);
        nPos = info.nTrackPos;
    }

    if (HIBYTE(nScrollCode)==SB_THUMBTRACK)
    {
        GetScrollInfo(SB_VERT, &info);
        nPos = info.nTrackPos;
    }

    return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}


