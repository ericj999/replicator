
// MFCApplication1Dlg.h : header file
//

#pragma once


// CMFCApplication1Dlg dialog
class CMFCApplication1Dlg : public CDialog
{
// Construction
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedIfiledialog();
	afx_msg void OnBnClickedFiledialog();
	afx_msg void OnBnClickedFolderpickerdialog();
	afx_msg void OnBnClickedShcreateitemfromparsingname();
	afx_msg void OnBnClickedGetPortableDevices();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedEnumFolderPath();
	afx_msg void OnBnClickedGetDevice();
	afx_msg void OnBnClickedOpenFolder();
	afx_msg void OnBnClickedRelativePath();
	afx_msg void OnBnClickedFilename();
	afx_msg void OnBnClickedParseparsingpath();
	afx_msg void OnBnClickedCreateFolder();
};
