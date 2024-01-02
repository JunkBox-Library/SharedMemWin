#pragma once

// NetSettingDLG ダイアログ

class NetSettingDLG : public CDialog
{
    DECLARE_DYNAMIC(NetSettingDLG)

public:
    NetSettingDLG(NetParam netparam, CWnd* pParent = NULL);   // 標準コンストラクタ
    virtual ~NetSettingDLG();

    virtual BOOL  OnInitDialog();

    NetParam    GetParameter();

// ダイアログ データ
    enum { IDD = IDD_NETSETTING_DIALOG };

protected:
    NetParam    netparam;

    BOOL        proxyMode;
    int         localPort;
    int         remotePort;
    CString     remoteHost;
    BOOL        binHexMode;
    BOOL        dummy1;
    BOOL        dummy2;

    CButton*    throwModeCBox;
    CButton*    proxyModeCBox;

    CButton*    binHexCBox;

    CEdit*      localPortEBox;
    CEdit*      remotePortEBox;
    CEdit*      remoteHostEBox;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnEnChangeEditRhost();
    afx_msg void OnBnClickedRadioProxy();
    afx_msg void OnBnClickedRadioThrow();
    afx_msg void OnBnClickedCheckBinhexMode();
};

