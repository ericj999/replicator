#pragma once
#include "resource.h"
#include "afxwin.h"

// CAdvancedOptionsDlg dialog

class CAdvancedOptionsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAdvancedOptionsDlg)

public:
	CAdvancedOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAdvancedOptionsDlg();

	void set_Flags(int flags) { m_flags = flags; }
	void set_Filters(LPCTSTR filters) { m_filters = filters; }
	void set_Options(int options) { m_options = options; }
	void set_DestinationFolderFormat(LPCTSTR folderFormat) { m_destinationFolderFormat = folderFormat;  }
	void setNoDestinationOptions() { m_noDestinationOptions = true; }

	int get_Flags() { return m_flags;  }
	CString get_Filters() { return m_filters; }
	int get_Options() { return m_options;  }
	CString get_DestinationFolderFormat() { return m_destinationFolderFormat; }

// Dialog Data
	enum { IDD = IDD_ADVANCED_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	int m_flags;
	int m_options;
	CString m_filters;
	CString m_destinationFolderFormat;
	bool m_noDestinationOptions;

	void UpdateFolderOption(int flags);

public:
	afx_msg void OnBnClickedNewTaskAllFiles();
	afx_msg void OnBnClickedNewTaskInclusion();
	afx_msg void OnBnClickedNewTaskExclusion();
	CComboBox m_cbUpdateOptions;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAoEditFormat();
	afx_msg void OnBnClickedDestinationGroup();
	afx_msg void OnBnClickedDestinationSameFromSelect();
	afx_msg void OnBnClickedDestinationSameFromRoot();
};
