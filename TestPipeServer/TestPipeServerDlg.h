
// TestPipeServerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <memory>

#include "MessageServer.h"


// CTestPipeServerDlg dialog
class CTestPipeServerDlg : public CDialogEx
{
// Construction
public:
	CTestPipeServerDlg(CWnd* pParent = NULL);	// standard constructor
	~CTestPipeServerDlg();

// Dialog Data
	enum { IDD = IDD_TESTPIPESERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	MessageServer m_server;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_pipeDirections;
	CEdit m_receive;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedTsdSendBtn();
	afx_msg void OnBnClickedTsdStartListening();
	afx_msg void OnBnClickedCancel();
};
