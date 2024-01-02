// MemSettingDLG.cpp : 実装ファイル
//

#include "stdafx.h"
#include "SharedMemWin.h"
#include "MemSettingDLG.h"

#include "TCP_thread.h"

using namespace jbxl;
//using namespace jbxwl;


// MemSettingDLG ダイアログ

IMPLEMENT_DYNAMIC(MemSettingDLG, CDialog)


MemSettingDLG::MemSettingDLG(NetParam param, CWnd* pParent /*=NULL*/)
    : CDialog(MemSettingDLG::IDD, pParent)
{
    netparam        = param;
    proxyMode       = netparam.proxymode;
    localPort       = netparam.sport;
    remoteHost      = netparam.hostname;
    remotePort      = netparam.cport;
    binHexMode      = netparam.binhexmode;

    throwModeCBox   = NULL;
    proxyModeCBox   = NULL;

    binHexCBox      = NULL;

    localPortEBox   = NULL;
    remotePortEBox  = NULL;
    remoteHostEBox  = NULL;
}


MemSettingDLG::~MemSettingDLG()
{
}


void MemSettingDLG::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MemSettingDLG, CDialog)
    ON_BN_CLICKED(IDC_RADIO_PROXY, &MemSettingDLG::OnBnClickedRadioProxy)
    ON_BN_CLICKED(IDC_RADIO_THROW, &MemSettingDLG::OnBnClickedRadioThrow)
END_MESSAGE_MAP()


BOOL   MemSettingDLG::OnInitDialog()
{
    char buf[LMNAME];

    throwModeCBox    = (CButton*)GetDlgItem(IDC_RADIO_THROW);
    proxyModeCBox    = (CButton*)GetDlgItem(IDC_RADIO_PROXY);

    binHexCBox       = (CButton*)GetDlgItem(IDC_CHECK_BINHEX_MODE);

    localPortEBox    = (CEdit*)GetDlgItem(IDC_EDIT_LPORT);
    remotePortEBox   = (CEdit*)GetDlgItem(IDC_EDIT_RPORT);
    remoteHostEBox   = (CEdit*)GetDlgItem(IDC_EDIT_RHOST);

    snprintf(buf, LMNAME-1, "%d", localPort);
    localPortEBox->SetWindowText(buf);

    if (proxyMode) {
        OnBnClickedRadioProxy();
    }
    else {
        OnBnClickedRadioThrow();
    }

    if (binHexMode) {
        binHexCBox->SetCheck(BST_CHECKED);
    }

    return TRUE;
}


void MemSettingDLG::OnOK() 
{
    char buf[LMNAME];

    if (proxyModeCBox->GetCheck()) proxyMode = TRUE;
    else                           proxyMode = FALSE;

    localPortEBox->GetWindowText(buf, LMNAME-1);
    localPort = atoi(buf);

    if (!proxyMode) {
        remotePortEBox->GetWindowText(buf, LMNAME-1);
        remotePort = atoi(buf);
        remoteHostEBox->GetWindowText(remoteHost);
    }

    if (binHexCBox->GetCheck()) binHexMode = TRUE;
    else                        binHexMode = FALSE;
    
    CDialog::OnOK();
}


NetParam  MemSettingDLG::GetParameter()
{
    NetParam  retparam;

    retparam            = netparam;
    retparam.sport      = localPort;
    retparam.hostname   = remoteHost;
    retparam.cport      = remotePort;
    retparam.proxymode  = proxyMode;
    retparam.binhexmode = binHexMode;

    return retparam;
}


void MemSettingDLG::OnBnClickedRadioThrow()
{
    char  buf[LMNAME];

    throwModeCBox->SetCheck(BST_CHECKED);
    proxyModeCBox->SetCheck(BST_UNCHECKED);

    snprintf(buf, LMNAME-1, "%d", remotePort);
    remotePortEBox->SetWindowText(buf);
    remoteHostEBox->SetWindowText(remoteHost);
    remotePortEBox->SetReadOnly(0);
    remoteHostEBox->SetReadOnly(0);
}


void MemSettingDLG::OnBnClickedRadioProxy()
{
    throwModeCBox->SetCheck(BST_UNCHECKED);
    proxyModeCBox->SetCheck(BST_CHECKED);

    remotePortEBox->SetWindowText("0");
    remoteHostEBox->SetWindowText("");
    remotePortEBox->SetReadOnly();
    remoteHostEBox->SetReadOnly();
}

