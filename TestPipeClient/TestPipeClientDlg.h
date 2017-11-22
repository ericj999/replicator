
// TestPipeClientDlg.h : header file
//

#pragma once
#include "NamedPipe.h"
#include "afxwin.h"
#include <thread>

// CTestPipeClientDlg dialog
class CTestPipeClientDlg : public CDialogEx
{
// Construction
public:
	CTestPipeClientDlg(CWnd* pParent = NULL);	// standard constructor
	~CTestPipeClientDlg();

// Dialog Data
	enum { IDD = IDD_TESTPIPECLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	std::unique_ptr<IPC::PipeClient> m_client;
	std::thread m_readThread;

	void ReadFunc();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedConnect();
	CComboBox m_pipeDirectionList;
	afx_msg void OnBnClickedTcdSend();
	CEdit m_receive;
};
