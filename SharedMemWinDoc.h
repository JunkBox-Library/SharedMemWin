// NetProtocolDoc.h : CNetProtocolDoc クラスのインターフェイス
//

#pragma once

#include   <Afxmt.h> 
#include  "BufferRing.h"

// 最大行数
#define  MAXBUFFERLINE 50000

class  CNetProtocolView;
class  CNetProtocolApp;

class CNetProtocolDoc : public CDocument
{
protected: // シリアル化からのみ作成します。
    CNetProtocolDoc();
    DECLARE_DYNCREATE(CNetProtocolDoc)

public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    CNetProtocolApp* pApp;

public:
    virtual ~CNetProtocolDoc();

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
    CNetProtocolView*  GetView(void);
};

