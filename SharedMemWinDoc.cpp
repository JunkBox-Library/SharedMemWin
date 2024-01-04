// SharedMemWinDoc.cpp : CSharedMemWinDoc クラスの実装
//

#include "stdafx.h"

#include "SharedMemWin.h"
#include "SharedMemWinDoc.h"
#include "WinTools.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace jbxl;
//using namespace jbxwl;


// CSharedMemWinDoc

IMPLEMENT_DYNCREATE(CSharedMemWinDoc, CDocument)

BEGIN_MESSAGE_MAP(CSharedMemWinDoc, CDocument)
END_MESSAGE_MAP()


// CSharedMemWinDoc コンストラクション/デストラクション

CSharedMemWinDoc::CSharedMemWinDoc()
{
    save_fname = "";
    pApp = NULL;

    bufferRing = new CBufferRing(MAXBUFFERLINE);
}


CSharedMemWinDoc::~CSharedMemWinDoc()
{
    //DEBUG_Error("ディストラクタ：IN  CSharedMemWinDoc");

    pApp->m_state = SHM_STOP;
    CSharedMemWinDoc::free();

    //DEBUG_Error("ディストラクタ：OUT CSharedMemWinDoc");
}


void  CSharedMemWinDoc::free(void)
{
    if (bufferRing!=NULL) {
        delete(bufferRing);
        bufferRing = NULL;
    }
}


BOOL CSharedMemWinDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument()) return FALSE;
    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// CSharedMemWinDoc シリアル化

void CSharedMemWinDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: 格納するコードをここに追加してください。
    }
    else
    {
        // TODO: 読み込むコードをここに追加してください。
    }
}



///////////////////////////////////////////////////////////////////////////////
// CSharedMemWinDoc 診断

#ifdef _DEBUG
void CSharedMemWinDoc::AssertValid() const
{
    CDocument::AssertValid();
}


void CSharedMemWinDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG



///////////////////////////////////////////////////////////////////////////////
// CSharedMemWinDoc コマンド

void CSharedMemWinDoc::DeleteContents(void)
{
    //CSharedMemWinDoc::clear();
    //DEBUG_Warning("CSharedMemWinDoc::DeleteContents: IN");
    CDocument::DeleteContents();
}


CSharedMemWinView*  CSharedMemWinDoc::GetView(void)
{
    POSITION pos = GetFirstViewPosition();
    while (pos!=NULL) {
        CSharedMemWinView* pview = (CSharedMemWinView*)GetNextView(pos);
        if (this==pview->GetDocument()) return pview;
    }
    return NULL;
}


void  CSharedMemWinDoc::clear(void)
{
    save_fname = "";

    CSingleLock lock(&criticalKey);
    lock.Lock();
    while (!lock.IsLocked()) {
        Sleep(100);
        lock.Lock();
    }
    
    bufferRing->clear();
    lock.Unlock();

    return;
}


int   CSharedMemWinDoc::writeLogFile(void)
{
    if (save_fname=="") return -1;

    CSingleLock lock(&criticalKey);
    lock.Lock();
    while (!lock.IsLocked()) {
        Sleep(100);
        lock.Lock();
    }
    
    int size = 0;
    //FILE* fp = fopen((LPCSTR)save_fname, "wb");
    FILE* fp = fopen(jbxwl::ts2mbs(save_fname), "wb");
    if (fp==NULL) {
        lock.Unlock();
        return -2;
    }

    for (int pos=0; pos<bufferRing->getMaxLineY(); pos++) {
        Buffer buf = bufferRing->pBuf[pos];
        //fwrite((const char*)buf.buf, buf.vldsz, 1, fp);
        fwrite((const char*)buf.buf, strlen((const char*)buf.buf), 1, fp);
        size += (int)strlen((const char*)buf.buf);
    }
    fclose(fp);
    lock.Unlock();

    return size;
}


CString  CSharedMemWinDoc::easyGetSaveFileName(LPCSTR title, HWND hWnd) 
{    
    OPENFILENAME  ofn;
    char fn[LNAME];
    CString  str = _T(""); 

    bzero(fn, LNAME);
    bzero(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.Flags = 0;
    ofn.lpstrFile = (LPWSTR)fn;
    ofn.nMaxFile  = LNAME;
    if (title!=NULL) ofn.lpstrTitle = (LPWSTR)title;

    BOOL ret = GetSaveFileName(&ofn);
    if (ret) str = fn;

    return  (LPCWSTR)fn;
}
