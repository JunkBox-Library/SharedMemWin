// MemSettingDLG.cpp : 実装ファイル
//

#include "stdafx.h"
#include "SharedMemWin.h"
#include "MemSettingDLG.h"

#include "SHM_thread.h"

using namespace jbxl;
//using namespace jbxwl;


// MemSettingDLG ダイアログ

IMPLEMENT_DYNAMIC(MemSettingDLG, CDialog)


MemSettingDLG::MemSettingDLG(ShmParam param, CWnd* pParent /*=NULL*/)
    : CDialog(MemSettingDLG::IDD, pParent)
{
    shmparam    = param;
    binHexMode  = shmparam.binhexmode;

    binHexCBox  = NULL;
}


MemSettingDLG::~MemSettingDLG()
{
}


void MemSettingDLG::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MemSettingDLG, CDialog)

END_MESSAGE_MAP()


BOOL   MemSettingDLG::OnInitDialog()
{
    binHexCBox = (CButton*)GetDlgItem(IDC_CHECK_BINHEX_MODE);

    if (binHexMode) {
        binHexCBox->SetCheck(BST_CHECKED);
    }

    return TRUE;
}


void MemSettingDLG::OnOK() 
{
    if (binHexCBox->GetCheck()) binHexMode = TRUE;
    else                        binHexMode = FALSE;
    
    CDialog::OnOK();
}


ShmParam  MemSettingDLG::GetParameter()
{
    ShmParam  retparam;

    retparam            = shmparam;
    retparam.binhexmode = binHexMode;

    return retparam;
}

