// NetProtocolDoc.cpp : CNetProtocolDoc クラスの実装
//

#include "stdafx.h"
#include "NetProtocol.h"

#include "NetProtocolDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace jbxl;
//using namespace jbxwl;


// CNetProtocolDoc

IMPLEMENT_DYNCREATE(CNetProtocolDoc, CDocument)

BEGIN_MESSAGE_MAP(CNetProtocolDoc, CDocument)
END_MESSAGE_MAP()


// CNetProtocolDoc コンストラクション/デストラクション

CNetProtocolDoc::CNetProtocolDoc()
{
    save_fname = "";
    pApp = NULL;

    bufferRing = new CBufferRing(MAXBUFFERLINE);
}


CNetProtocolDoc::~CNetProtocolDoc()
{
    //DEBUG_Error("ディストラクタ：IN  CNetProtocolDoc");

    pApp->m_state = RELAY_STOP;
    CNetProtocolDoc::free();

    //DEBUG_Error("ディストラクタ：OUT CNetProtocolDoc");
}


void  CNetProtocolDoc::free(void)
{
    if (bufferRing!=NULL) {
        delete(bufferRing);
        bufferRing = NULL;
    }
}


BOOL CNetProtocolDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument()) return FALSE;
    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// CNetProtocolDoc シリアル化

void CNetProtocolDoc::Serialize(CArchive& ar)
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
// CNetProtocolDoc 診断

#ifdef _DEBUG
void CNetProtocolDoc::AssertValid() const
{
    CDocument::AssertValid();
}


void CNetProtocolDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG



///////////////////////////////////////////////////////////////////////////////
// CNetProtocolDoc コマンド

void CNetProtocolDoc::DeleteContents(void)
{
    //CNetProtocolDoc::clear();
    //DEBUG_Warning("CNetProtocolDoc::DeleteContents: IN");
    CDocument::DeleteContents();
}


CNetProtocolView*  CNetProtocolDoc::GetView(void)
{
    POSITION pos = GetFirstViewPosition();
    while (pos!=NULL) {
        CNetProtocolView* pview = (CNetProtocolView*)GetNextView(pos);
        if (this==pview->GetDocument()) return pview;
    }
    return NULL;
}


void  CNetProtocolDoc::clear(void)
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


int   CNetProtocolDoc::writeLogFile(void)
{
    if (save_fname=="") return -1;

    CSingleLock lock(&criticalKey);
    lock.Lock();
    while (!lock.IsLocked()) {
        Sleep(100);
        lock.Lock();
    }
    
    int size = 0;
    FILE* fp = fopen((LPCSTR)save_fname, "wb");
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


CString  CNetProtocolDoc::easyGetSaveFileName(LPCSTR title, HWND hWnd) 
{    
    OPENFILENAME  ofn;
    char fn[LNAME];
    CString  str=""; 

    bzero(fn, LNAME);
    bzero(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.Flags = 0;
    ofn.lpstrFile = fn;
    ofn.nMaxFile  = LNAME;
    if (title!=NULL) ofn.lpstrTitle = title;

    BOOL ret = GetSaveFileName(&ofn);
    if (ret) str = fn;

    return fn;
}
