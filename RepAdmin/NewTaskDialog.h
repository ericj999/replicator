#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "Property.h"

// CNewTaskDialog dialog

class CNewTaskDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CNewTaskDialog)

public:
	CNewTaskDialog(CWnd* pParent = NULL);   // standard constructor
	CNewTaskDialog(const Database::PropertyList& propList, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewTaskDialog();

// Dialog Data
	enum { IDD = IDD_NEW_TASK };

	LPCTSTR GetNewTaskName() { return m_strNewTaskName; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	bool m_update;
	bool m_changed;
	bool m_ready;
	Database::PropertyList m_props;
	void UpdateControls();
	void UpdateCreateButtons();
	void setChanged() { if (m_ready) m_changed = true; }

	// advanced options
	int m_AdvnacedOptionsFlags;
	CString m_strFilters;
	CString m_strDestinationFolderFormat;
	CString m_strSourceParsing;
	bool m_portableSource;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedNewtaskBrowseDest();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChange();
	afx_msg void OnEnChangeNewTaskDestination();
	CEdit m_editName;
	CEdit m_editSource;
	CEdit m_editDest;
	CString m_strNewTaskName;
	afx_msg void OnBnClickedNewTaskCreate();
	afx_msg void OnBnClickedNewTaskAdvOptions();
	afx_msg void OnBnClickedNewTaskIncludeSub();
	afx_msg void OnBnClickedNewTaskBrowseSource();
};
