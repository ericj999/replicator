#pragma once


// CSettingsPage dialog

class CSettingsPage : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsPage)

public:
	CSettingsPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsPage();

// Dialog Data
	enum { IDD = IDD_SETTINGSPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
