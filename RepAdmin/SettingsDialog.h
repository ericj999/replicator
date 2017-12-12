#pragma once
#include "afxwin.h"


// SettingsDialog dialog

class SettingsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(SettingsDialog)

public:
	SettingsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SettingsDialog();

// Dialog Data
	enum { IDD = IDD_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int m_historyDays;
	afx_msg void OnBnClickedOk();
};
