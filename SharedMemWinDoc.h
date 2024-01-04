// SharedMemWinDoc.h : CSharedMemWinDoc クラスのインターフェイス
//

#pragma once

#include   <afxmt.h> 
#include  "BufferRing.h"

// 最大行数
#define  MAXBUFFERLINE 50000

class  CSharedMemWinView;
class  CSharedMemWinApp;

class CSharedMemWinDoc : public CDocument
{
protected: // シリアル化からのみ作成します。
    CSharedMemWinDoc();
    DECLARE_DYNCREATE(CSharedMemWinDoc)

public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    CSharedMemWinApp* pApp;

public:
    virtual ~CSharedMemWinDoc();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

// 生成された、メッセージ割り当て関数
protected:
    DECLARE_MESSAGE_MAP()

public:
    CString             save_fname;
    CBufferRing*        bufferRing;
    CCriticalSection    criticalKey;

public:
    void        free(void);
    void        clear(void);
    int         writeLogFile(void);
    CString     easyGetSaveFileName(LPCSTR title, HWND hWnd);

    void        DeleteContents(void);
    CSharedMemWinView*  GetView(void);
};

