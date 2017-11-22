
// TestPipeServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include <thread>
#include <functional>
#include "TestPipeServer.h"
#include "TestPipeServerDlg.h"
#include "afxdialogex.h"

#include "NamedPipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestPipeServerDlg dialog



CTestPipeServerDlg::CTestPipeServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestPipeServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTestPipeServerDlg::~CTestPipeServerDlg()
{
	m_server.Stop();
}

void CTestPipeServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TSD_DIRECTIONS, m_pipeDirections);
	DDX_Control(pDX, IDC_TSD_RECEIVE_MSGS, m_receive);
}

BEGIN_MESSAGE_MAP(CTestPipeServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CTestPipeServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_TSD_SEND_BTN, &CTestPipeServerDlg::OnBnClickedTsdSendBtn)
	ON_BN_CLICKED(IDC_TSD_START_LISTENING, &CTestPipeServerDlg::OnBnClickedTsdStartListening)
	ON_BN_CLICKED(IDCANCEL, &CTestPipeServerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTestPipeServerDlg message handlers

BOOL CTestPipeServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetDlgItemInt(IDC_TSD_INSTNUM, 1);
	m_pipeDirections.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestPipeServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestPipeServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestPipeServerDlg::OnBnClickedOk()
{
}


void CTestPipeServerDlg::OnBnClickedTsdSendBtn()
{
	if (m_server.IsRunning())
	{
		CString str;

		GetDlgItemText(IDC_TSD_SEND_MSG, str);
		if (!str.IsEmpty())
		{
			size_t size = (str.GetLength() + 1) * sizeof(TCHAR);
			m_server.Broadcast((void*)((LPCTSTR)str), size);
		}
	}
}

void ServerCallback(void* p, const unsigned char* data, size_t size)
{
	StringT msg;
	if (data && (size > 0))
		msg = reinterpret_cast<TCHAR*>(const_cast<unsigned char*>(data));

	CTestPipeServerDlg* dlg = static_cast<CTestPipeServerDlg*>(p);
	if (dlg)
	{
		int n = dlg->m_receive.GetWindowTextLength();
		if ((n + msg.length()) > 64 * 1024)
		{
			dlg->m_receive.Clear();
			n = 0;
		}
		dlg->m_receive.SetSel(n, n);
		dlg->m_receive.ReplaceSel(msg.c_str());
	}
}

void CTestPipeServerDlg::OnBnClickedTsdStartListening()
{
	GetDlgItem(IDC_TSD_START_LISTENING)->EnableWindow(FALSE);
	m_pipeDirections.EnableWindow(FALSE);
	GetDlgItem(IDC_TSD_INSTNUM)->EnableWindow(FALSE);

	int instanceNumber = static_cast<int>(GetDlgItemInt(IDC_TSD_INSTNUM));
	IPC::PipeDirection direction;
	switch (m_pipeDirections.GetCurSel())
	{
	case 1: direction = IPC::PipeDirection::out; break;
	case 2: direction = IPC::PipeDirection::both; break;
	default: direction = IPC::PipeDirection::in; break;
	}

	try
	{
		m_server.Start(_T("\\\\.\\pipe\\__TestReplicatorPipe__"), instanceNumber, direction, std::bind(ServerCallback, this, std::placeholders::_1, std::placeholders::_2));
	}
	catch (IPC::Exception& e)
	{
		CString str(e.what());
		AfxMessageBox(str);
		GetDlgItem(IDC_TSD_START_LISTENING)->EnableWindow(TRUE);
		m_pipeDirections.EnableWindow(TRUE);
		GetDlgItem(IDC_TSD_INSTNUM)->EnableWindow(TRUE);
		return;
	}
	BOOL enableSend = ((direction == IPC::PipeDirection::out) || (direction == IPC::PipeDirection::both)) ? TRUE : FALSE;
	GetDlgItem(IDC_TSD_SEND_BTN)->EnableWindow(enableSend);
	GetDlgItem(IDC_TSD_SEND_MSG)->EnableWindow(enableSend);
}


void CTestPipeServerDlg::OnBnClickedCancel()
{
	m_server.Stop();
	CDialogEx::OnCancel();
}
