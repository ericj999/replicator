
// TestPipeClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestPipeClient.h"
#include "TestPipeClientDlg.h"
#include "afxdialogex.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestPipeClientDlg dialog



CTestPipeClientDlg::CTestPipeClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestPipeClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTestPipeClientDlg::~CTestPipeClientDlg()
{
	if (m_readThread.joinable())
		m_readThread.join();
}

void CTestPipeClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TCD_DIRECTIONS, m_pipeDirectionList);
	DDX_Control(pDX, IDC_RECEIVE, m_receive);
}

BEGIN_MESSAGE_MAP(CTestPipeClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CTestPipeClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CONNECT, &CTestPipeClientDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_TCD_SEND, &CTestPipeClientDlg::OnBnClickedTcdSend)
END_MESSAGE_MAP()


// CTestPipeClientDlg message handlers

BOOL CTestPipeClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_pipeDirectionList.SetCurSel(0);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestPipeClientDlg::OnPaint()
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
HCURSOR CTestPipeClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestPipeClientDlg::OnBnClickedOk()
{
}


void CTestPipeClientDlg::OnBnClickedConnect()
{
	IPC::PipeDirection direction;
	GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_TCD_DIRECTIONS)->EnableWindow(FALSE);

	switch (m_pipeDirectionList.GetCurSel())
	{
	case 1: direction = IPC::PipeDirection::both; break;
	case 2: direction = IPC::PipeDirection::in; break;
	default: direction = IPC::PipeDirection::out; break;
	}

	m_client = std::make_unique<IPC::PipeClient>(_T("\\\\.\\pipe\\__TestReplicatorPipe__"), direction);
	if (m_client->Connect())
	{
		if ((direction == IPC::PipeDirection::both) || (direction == IPC::PipeDirection::in))
			m_readThread = std::thread(&CTestPipeClientDlg::ReadFunc, this);

		if ((direction == IPC::PipeDirection::both) || (direction == IPC::PipeDirection::out))
			GetDlgItem(IDC_TCD_SEND)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_TCD_DIRECTIONS)->EnableWindow(TRUE);
	}
}


void CTestPipeClientDlg::OnBnClickedTcdSend()
{
	if (m_client)
	{
		CString str;

		GetDlgItemText(IDC_TCD_SEND_MSG, str);

		size_t size = (str.GetLength() + 1) * sizeof(TCHAR);
		IPC::PipeData data(size);
		memcpy(data.data(), (LPCTSTR)str, size);
		try
		{
			m_client->Write(data);
		}
		catch (IPC::Exception e)
		{
			CString err{ e.what() };

			AfxMessageBox(err);
		}
	}
}

void CTestPipeClientDlg::ReadFunc()
{
	try
	{
		while (true)
		{
			IPC::PipeData pd = m_client->Read(INFINITE);

			StringT msg;
			if (pd.size() > 0)
			{
				msg = reinterpret_cast<TCHAR*>(const_cast<unsigned char*>(pd.data()));

				int n = m_receive.GetWindowTextLength();
				if ((n + msg.length()) > 64 * 1024)
				{
					m_receive.Clear();
					n = 0;
				}
				m_receive.SetSel(n, n);
				m_receive.ReplaceSel(msg.c_str());
			}
		}
	}
	catch (IPC::Exception& e)
	{

	}
}