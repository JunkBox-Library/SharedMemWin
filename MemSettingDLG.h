#pragma once

// MemSettingDLG ダイアログ

class MemSettingDLG : public CDialog
{
    DECLARE_DYNAMIC(MemSettingDLG)

public:
    MemSettingDLG(ShmParam p_shmparam, CWnd* pParent = NULL);   // 標準コンストラクタ
    virtual ~MemSettingDLG();

    virtual BOOL  OnInitDialog();

    ShmParam    GetParameter();

// ダイアログ データ
    enum { IDD = IDD_NETSETTING_DIALOG };

protected:
    ShmParam    shmparam;

    BOOL        binHexMode;
    //BOOL        dummy1;
    //BOOL        dummy2;

    CButton*    binHexCBox;

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

