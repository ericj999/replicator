#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// FolderFormatDialog dialog

class FolderFormatDialog : public CDialogEx
{
	DECLARE_DYNAMIC(FolderFormatDialog)

public:
	FolderFormatDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~FolderFormatDialog();

	void set_format(LPCTSTR format) { m_format = format; }
	CString get_format() { return m_format;  }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FOLDER_FORMATTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_format;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAfInsert();
	CListCtrl m_macros;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedAfMacros(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeAfFormat();
	CEdit m_editFormat;
};
