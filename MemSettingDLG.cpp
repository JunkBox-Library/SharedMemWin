// NetSettingDLG.cpp : 実装ファイル
//

#include "stdafx.h"
#include "NetProtocol.h"
#include "NetSettingDLG.h"

#include "TCP_thread.h"

using namespace jbxl;
//using namespace jbxwl;


// NetSettingDLG ダイアログ

IMPLEMENT_DYNAMIC(NetSettingDLG, CDialog)


NetSettingDLG::NetSettingDLG(NetParam param, CWnd* pParent /*=NULL*/)
    : CDialog(NetSettingDLG::IDD, pParent)
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


NetSettingDLG::~NetSettingDLG()
{
}


void NetSettingDLG::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(NetSettingDLG, CDialog)
    ON_BN_CLICKED(IDC_RADIO_PROXY, &NetSettingDLG::OnBnClickedRadioProxy)
    ON_BN_CLICKED(IDC_RADIO_THROW, &NetSettingDLG::OnBnClickedRadioThrow)
END_MESSAGE_MAP()


BOOL   NetSettingDLG::OnInitDialog()
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


void NetSettingDLG::OnOK() 
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


NetParam  NetSettingDLG::GetParameter()
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


void NetSettingDLG::OnBnClickedRadioThrow()
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


void NetSettingDLG::OnBnClickedRadioProxy()
{
    throwModeCBox->SetCheck(BST_UNCHECKED);
    proxyModeCBox->SetCheck(BST_CHECKED);

    remotePortEBox->SetWindowText("0");
    remoteHostEBox->SetWindowText("");
    remotePortEBox->SetReadOnly();
    remoteHostEBox->SetReadOnly();
}

