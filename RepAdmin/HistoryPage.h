#pragma once


// CHistoryPage dialog

class CHistoryPage : public CDialogEx
{
	DECLARE_DYNAMIC(CHistoryPage)

public:
	CHistoryPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHistoryPage();

// Dialog Data
	enum { IDD = IDD_HISTORYPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
