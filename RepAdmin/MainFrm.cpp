
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Replicator.h"

#include "MainFrm.h"
#include "TaskListView.h"
#include "ReplicatorView.h"

#include "GlobDef.h"
#include "DBDef.h"
#include "Recordset.h"
#include "Table.h"
#include "Property.h"

#include "SettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define REGSTR_KEY_CONFIG_MAINFRAME _T("SOFTWARE\\Replicator\\Config\\MainFrame")
#define REGSTR_VALUE_LEFT	_T("Left")
#define REGSTR_VALUE_RIGHT	_T("Right")
#define REGSTR_VALUE_TOP	_T("Top")
#define REGSTR_VALUE_BOTTOM	_T("Bottom")

#define ABORT_WAIT_ELAPSE_TIME	1000	// 1 second
#define IDT_WAIT_EXIT			1000
#define WAIT_EXIT_COUNTER		60		// 60 seconds

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

//const int  iMaxUserToolbars = 10;
//const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
//const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_TASK_DELETE, &CMainFrame::OnTaskDelete)
	ON_COMMAND(ID_TASK_NEW, &CMainFrame::OnTaskNew)
	ON_COMMAND(ID_TASK_RUN, &CMainFrame::OnTaskRun)
	ON_UPDATE_COMMAND_UI(ID_TASK_RUN, &CMainFrame::OnUpdateTaskRun)
	ON_UPDATE_COMMAND_UI(ID_TASK_DELETE, &CMainFrame::OnUpdateTaskDelete)
	ON_COMMAND(ID_TASK_EDIT, &CMainFrame::OnTaskEdit)
	ON_UPDATE_COMMAND_UI(ID_TASK_EDIT, &CMainFrame::OnUpdateTaskEdit)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_COMMAND(ID_TASK_STOP, &CMainFrame::OnTaskStop)
	ON_UPDATE_COMMAND_UI(ID_TASK_STOP, &CMainFrame::OnUpdateTaskStop)
	ON_WM_TIMER()
	ON_COMMAND(ID_TOOLS_SETTINGS, &CMainFrame::OnToolsSettings)
	ON_COMMAND(ID_HELP_VIEWHELP, &CMainFrame::OnHelpViewhelp)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR	//,           // status line indicator
//	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
	m_waitDialog(NULL), m_timerId(0), m_waitExitCounter(0)
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
	if (m_waitDialog)
	{
		m_waitDialog->DestroyWindow();
		delete m_waitDialog;
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	// Allow user-defined toolbars operations:
//	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	//m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_TASK_RUN);
	lstBasicCommands.AddTail(ID_TASK_NEW);
	lstBasicCommands.AddTail(ID_TASK_EDIT);
	lstBasicCommands.AddTail(ID_TASK_DELETE);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_APP_ABOUT);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);
	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 2, 1))
		return FALSE;

	CRect rect;
	GetClientRect(&rect);

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CTaskListView), CSize(rect.Width(), rect.Height()/3), pContext) ||
		!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CReplicatorView), CSize(rect.Width(), rect.Height() - rect.Height()/3), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~(LONG)FWS_ADDTOTITLE;

	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}
	return TRUE;
}



void CMainFrame::OnTaskDelete()
{
	CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
	if (pView)
		pView->OnTaskDelete();
}

void CMainFrame::OnTaskNew()
{
	CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
	if (pView)
		pView->OnTaskNew();
}

void CMainFrame::OnTaskRun()
{
	CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
	if (pView)
		pView->OnTaskRun();
}


void CMainFrame::OnUpdateTaskRun(CCmdUI *pCmdUI)
{
	if (pCmdUI)
	{
		CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
		pCmdUI->Enable((pView && (pView->GetListCtrl().GetSelectedCount() > 0)) ? TRUE : FALSE);
	}
}


void CMainFrame::OnUpdateTaskDelete(CCmdUI *pCmdUI)
{
	if (pCmdUI)
	{
		CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
		pCmdUI->Enable((pView && (pView->GetListCtrl().GetSelectedCount() > 0)) ? TRUE : FALSE);
	}
}

void CMainFrame::Refresh(int taskID, DWORD refresh, bool force /*=false*/)
{
	CReplicatorView* pView = static_cast<CReplicatorView*>(m_wndSplitter.GetPane(1, 0));
	if (pView)
		pView->Refresh(taskID, refresh, force);
}



void CMainFrame::OnTaskEdit()
{
	CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
	if (pView)
		pView->OnTaskEdit();
}


void CMainFrame::OnUpdateTaskEdit(CCmdUI *pCmdUI)
{
	if (pCmdUI)
	{
		CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
		pCmdUI->Enable((pView && (pView->GetListCtrl().GetSelectedCount() > 0)) ? TRUE : FALSE);
	}
}


void CMainFrame::OnDestroy()
{
	SaveSettings();
	CFrameWndEx::OnDestroy();
}

void CMainFrame::LoadSettings()
{
	WINDOWPLACEMENT wpl;
	wpl.length = sizeof(wpl);
	if (GetWindowPlacement(&wpl))
	{
		HKEY hKey = NULL;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_MAINFRAME, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			DWORD value = static_cast<DWORD>(wpl.rcNormalPosition.left);
			DWORD size = static_cast<DWORD>(sizeof(DWORD));
			if (RegQueryValueEx(hKey, REGSTR_VALUE_LEFT, 0, NULL, LPBYTE(&value), &size) == ERROR_SUCCESS)
				wpl.rcNormalPosition.left = static_cast<LONG>(value);

			value = static_cast<DWORD>(wpl.rcNormalPosition.top);
			size = static_cast<DWORD>(sizeof(DWORD));
			if (RegQueryValueEx(hKey, REGSTR_VALUE_TOP, 0, NULL, LPBYTE(&value), &size) == ERROR_SUCCESS)
				wpl.rcNormalPosition.top = static_cast<LONG>(value);

			value = static_cast<DWORD>(wpl.rcNormalPosition.right);
			size = static_cast<DWORD>(sizeof(DWORD));
			if (RegQueryValueEx(hKey, REGSTR_VALUE_RIGHT, 0, NULL, LPBYTE(&value), &size) == ERROR_SUCCESS)
				wpl.rcNormalPosition.right = static_cast<LONG>(value);

			value = static_cast<DWORD>(wpl.rcNormalPosition.bottom);
			size = static_cast<DWORD>(sizeof(DWORD));
			if (RegQueryValueEx(hKey, REGSTR_VALUE_BOTTOM, 0, NULL, LPBYTE(&value), &size) == ERROR_SUCCESS)
				wpl.rcNormalPosition.bottom = static_cast<LONG>(value);

			RegCloseKey(hKey);
			SetWindowPlacement(&wpl);
		}
	}
}

void CMainFrame::SaveSettings()
{
	WINDOWPLACEMENT wpl;
	wpl.length = sizeof(wpl);
	if (GetWindowPlacement(&wpl))
	{
		DWORD disp = 0;
		HKEY hKey = NULL;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_MAINFRAME, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &disp) == ERROR_SUCCESS)
		{
			DWORD value = static_cast<DWORD>(wpl.rcNormalPosition.left);
			if (RegSetValueEx(hKey, REGSTR_VALUE_LEFT, 0, REG_DWORD, LPBYTE(&value), sizeof(DWORD)) == ERROR_SUCCESS)
				wpl.rcNormalPosition.left = static_cast<LONG>(value);

			value = static_cast<DWORD>(wpl.rcNormalPosition.top);
			if (RegSetValueEx(hKey, REGSTR_VALUE_TOP, 0, REG_DWORD, LPBYTE(&value), sizeof(DWORD)) == ERROR_SUCCESS)
				wpl.rcNormalPosition.top = static_cast<LONG>(value);

			value = static_cast<DWORD>(wpl.rcNormalPosition.right);
			if (RegSetValueEx(hKey, REGSTR_VALUE_RIGHT, 0, REG_DWORD, LPBYTE(&value), sizeof(DWORD)) == ERROR_SUCCESS)
				wpl.rcNormalPosition.right = static_cast<LONG>(value);

			value = static_cast<DWORD>(wpl.rcNormalPosition.bottom);
			if (RegSetValueEx(hKey, REGSTR_VALUE_BOTTOM, 0, REG_DWORD, LPBYTE(&value), sizeof(DWORD)) == ERROR_SUCCESS)
				wpl.rcNormalPosition.bottom = static_cast<LONG>(value);

			RegCloseKey(hKey);
		}
	}
}

//void CMainFrame::ServerCallback(void* _This, const unsigned char* data, size_t size)
//{
//	if (_This)
//	{
//		CMainFrame* mainFrame = static_cast<CMainFrame*>(_This);
//
//	}
//}


void CMainFrame::OnClose()
{
	CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
	if (pView)
	{
		if (pView->IsBusy())
		{
			if (AfxMessageBox(IDS_PROMPT_CANCEL_JOB, MB_YESNO) == IDNO)
				return;
			else
			{
				m_waitDialog = new WaitDialog(this);

				EnableWindow(FALSE);
				pView->StopAllTasks();

				m_timerId = SetTimer(IDT_WAIT_EXIT, ABORT_WAIT_ELAPSE_TIME, NULL);
				if (m_timerId)
				{
					m_waitExitCounter = 0;
					m_waitDialog->Create(WaitDialog::IDD);
					m_waitDialog->ShowWindow(SW_SHOWNORMAL);
					return;
				}
			}
		}
	}
	CFrameWndEx::OnClose();
}


void CMainFrame::OnTaskStop()
{
	CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
	if (pView)
		pView->OnTaskStop();
}


void CMainFrame::OnUpdateTaskStop(CCmdUI *pCmdUI)
{
	if (pCmdUI)
	{
		CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
		pCmdUI->Enable((pView && pView->IsSelectedTaskRunning()) ? TRUE : FALSE);
	}
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_timerId)
	{
		++m_waitExitCounter;
		CTaskListView* pView = static_cast<CTaskListView*>(m_wndSplitter.GetPane(0, 0));
		bool busy = (pView && pView->IsBusy()) ? true : false;
		if(!busy || (m_waitExitCounter > WAIT_EXIT_COUNTER))
		{
			KillTimer(m_timerId);
			m_timerId = 0;
			CFrameWndEx::OnClose();
		}
		return;
	}
	CFrameWndEx::OnTimer(nIDEvent);
}


void CMainFrame::OnToolsSettings()
{
	SettingsDialog dialog(this);

	dialog.m_historyDays = theApp.getHistoryDays();
	dialog.m_testRun = theApp.getTestRunMode();

	if (dialog.DoModal() == IDOK)
	{
		theApp.setHistoryDays(dialog.m_historyDays);
		theApp.setTestRunMode(dialog.m_testRun);

		CString title;
		if (dialog.m_testRun)
		{
			title.LoadString(IDS_TITLE_TESTRUN);
		}
		else
		{
			CString str;
			str.LoadString(IDR_MAINFRAME);
			int pos = str.Find(L'\n');
			if (pos > 0)
				title = str.Left(pos);
			else
				title = str;
		}
		SetWindowText(title);
	}
}

void CMainFrame::OnHelpViewhelp()
{
	CString url;
	url.LoadString(IDS_HELP_URL);
	ShellExecute(NULL, nullptr, url, nullptr, nullptr, SW_SHOW);
}
