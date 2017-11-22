
// LeftView.cpp : implementation of the CTaskListView class
//

#include "stdafx.h"
#include "resource.h"
#include "Replicator.h"
#include "MainFrm.h"
#include "ReplicatorDoc.h"
#include "TaskListView.h"
#include "DBDef.h"
#include "Table.h"
#include "Property.h"
#include "NewTaskDialog.h"
#include "util.h"
#include "GlobDef.h"

#include "RepRunner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define REGSTR_KEY_CONFIG_TASKVIEW _T("SOFTWARE\\Replicator\\Config\\TaskView")
#define STR_TASKLIST_CW _T("TaskListCW")

#define WM_TASK_DONE (WM_APP + 1)

enum
{
	LIST_COL_NAME = 0,
	LIST_COL_LAST_RUN,
	LIST_COL_LAST_SUCCESSFUL_RUN,
	LIST_COL_STATUS,
	LIST_COL_TASKID,
	MAX_UI_LIST_COLS = LIST_COL_TASKID,
	MAX_LIST_COLS
};

struct tagColumnDef
{
	int ids;
	int width;
	LPCTSTR colDBName;
	Database::PropertyType colDBType;
} ColumnDef[MAX_LIST_COLS] =
{
	{ IDS_LIST_NAME, 120, TASKS_COL_NAME, Database::PT_TEXT },
	{ IDS_LIST_LAST_RUN, 120, TASKS_COL_LASTRUN, Database::PT_TEXT },
	{ IDS_LIST_LAST_SUCCESSFUL_RUN, 120, TASKS_COL_LASTSUCCESSRUN, Database::PT_TEXT },
	{ IDS_LIST_STATUS, 480, TASKS_COL_LASTRUNSTATUS, Database::PT_TEXT },
	{ IDS_LIST_TASKID, 60, TASKS_COL_TASKID, Database::PT_INT64 }
};

// CTaskListView

IMPLEMENT_DYNCREATE(CTaskListView, CListView)

BEGIN_MESSAGE_MAP(CTaskListView, CListView)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CTaskListView::OnLvnItemchanged)
	ON_UPDATE_COMMAND_UI(ID_TASK_DELETE, &CTaskListView::OnUpdateTaskDelete)
	ON_UPDATE_COMMAND_UI(ID_TASK_NEW, &CTaskListView::OnUpdateTaskNew)
	ON_UPDATE_COMMAND_UI(ID_TASK_RUN, &CTaskListView::OnUpdateTaskRun)
	ON_COMMAND(ID_TASK_NEW, &CTaskListView::OnTaskNew)
	ON_COMMAND(ID_TASK_RUN, &CTaskListView::OnTaskRun)
	ON_COMMAND(ID_TASK_DELETE, &CTaskListView::OnTaskDelete)
	ON_COMMAND(ID_TASK_EDIT, &CTaskListView::OnTaskEdit)
	ON_UPDATE_COMMAND_UI(ID_TASK_EDIT, &CTaskListView::OnUpdateTaskEdit)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TASK_DONE, &CTaskListView::OnTaskDone)
	ON_COMMAND(ID_TASK_STOP, &CTaskListView::OnTaskStop)
	ON_UPDATE_COMMAND_UI(ID_TASK_STOP, &CTaskListView::OnUpdateTaskStop)
END_MESSAGE_MAP()


// CTaskListView construction/destruction

CTaskListView::CTaskListView()
{
	// TODO: add construction code here
}

CTaskListView::~CTaskListView()
{
}

BOOL CTaskListView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~(LVS_ICON | LVS_SMALLICON | LVS_LIST);
	cs.style |= LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;

	return CListView::PreCreateWindow(cs);
}

void CTaskListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
}


// CTaskListView diagnostics

#ifdef _DEBUG
void CTaskListView::AssertValid() const
{
	CListView::AssertValid();
}

void CTaskListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CReplicatorDoc* CTaskListView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReplicatorDoc)));
	return (CReplicatorDoc*)m_pDocument;
}
#endif //_DEBUG


// CTaskListView message handlers


int CTaskListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD dwStyles = GetListCtrl().GetExtendedStyle();
	dwStyles |= LVS_EX_FULLROWSELECT;
	GetListCtrl().SetExtendedStyle(dwStyles);
	GetListCtrl().ModifyStyle(0, LVS_SORTASCENDING);

	LoadSettings();

	Database::PropertyList props;
	for (int i = LIST_COL_NAME; i < MAX_LIST_COLS; ++i)
	{
		if (i < MAX_UI_LIST_COLS)
		{
			CString str;
			str.LoadString(ColumnDef[i].ids);
			GetListCtrl().InsertColumn(i, str, LVCFMT_LEFT, ColumnDef[i].width);
		}
		props.push_back(Database::Property(ColumnDef[i].colDBName, ColumnDef[i].colDBType));
	}

	Database::Table tb{ theApp.GetDB(), TASKS_TABLE };
	try
	{
		Database::RecordsetPtr rs = tb.Select(props);
		while (rs->Step())
			InsertItem(rs);
	}
	catch (Database::Exception e)
	{
		CString msg(e.what());
		AfxMessageBox(msg, MB_OK | MB_ICONEXCLAMATION);
	}
	return 0;
}


void CTaskListView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_TASK_PUPUP_MENU, point.x, point.y, this, TRUE);
}

void CTaskListView::AddNewTask(LPCTSTR taskName)
{
	Database::Table tb{ theApp.GetDB(), TASKS_TABLE };

	StringT condition = TASKS_COL_NAME;
	condition += _T("='") + String::replace(taskName, _T("'"), _T("''")) + _T("'");

	Database::PropertyList props;
	for (int i = LIST_COL_NAME; i < MAX_LIST_COLS; ++i)
		props.push_back(Database::Property(ColumnDef[i].colDBName, ColumnDef[i].colDBType));

	Database::RecordsetPtr rs = tb.Select(props, condition);
	if (rs->Step())
		InsertItem(rs);
}

int CTaskListView::InsertItem(Database::RecordsetPtr rs)
{
	int item = -1;
	if (rs)
	{
		int taskId = rs->GetColumnInt64(LIST_COL_TASKID, -1);
		if ((rs->GetColumnCount() == MAX_LIST_COLS) && (taskId != -1))
		{
			StringT name = rs->GetColumnStr(LIST_COL_NAME);
			if (!name.empty())
			{
				item = GetListCtrl().InsertItem(0, name.c_str());
				if (item != -1)
				{
					GetListCtrl().SetItemText(item, LIST_COL_LAST_RUN, String::UTCTimeToLocalTime(rs->GetColumnStr(LIST_COL_LAST_RUN)).c_str());
					GetListCtrl().SetItemText(item, LIST_COL_LAST_SUCCESSFUL_RUN, String::UTCTimeToLocalTime(rs->GetColumnStr(LIST_COL_LAST_SUCCESSFUL_RUN)).c_str());
					GetListCtrl().SetItemText(item, LIST_COL_STATUS, rs->GetColumnStr(LIST_COL_STATUS).c_str());
					GetListCtrl().SetItemData(item, taskId);
				}
			}
		}
	}
	return item;
}

void CTaskListView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	int taskId = GetSelectedTask();
	if (taskId > 0)
	{
		LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
		CMainFrame* pMain = static_cast<CMainFrame*>(theApp.GetMainWnd());
		pMain->Refresh(taskId);
	}
	*pResult = 0;
}

int CTaskListView::GetSelectedTask()
{
	int task = -1;
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		int item = GetListCtrl().GetNextSelectedItem(pos);
		task = static_cast<int>(GetListCtrl().GetItemData(item));
	}
	return task;
}

void CTaskListView::OnUpdateTaskDelete(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() > 0);
}


void CTaskListView::OnUpdateTaskNew(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void CTaskListView::OnUpdateTaskRun(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() > 0);
}


void CTaskListView::OnTaskNew()
{
	CNewTaskDialog dlg(this);

	if (dlg.DoModal() == IDOK)
		AddNewTask(dlg.GetNewTaskName());
}

void CTaskListView::OnTaskRun()
{
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		int item = GetListCtrl().GetNextSelectedItem(pos);
		int taskId = static_cast<int>(GetListCtrl().GetItemData(item));

		CString state;
		state.LoadString(IDS_TASK_RUNNING);
		GetListCtrl().SetItemText(item, LIST_COL_LAST_RUN, state);

		CReplicatorApp* app = static_cast<CReplicatorApp*>(AfxGetApp());
		bool verbose = app ? app->getVerboseMode() : false;
		bool testRun = app ? app->getTestRunMode() : false;

		std::lock_guard<std::mutex> lock{m_tasksLock};
		std::shared_ptr<RepRunner> runner{ new RepRunner{ taskId, std::bind(&CTaskListView::EventCallback, this, taskId, std::placeholders::_1, std::placeholders::_2), verbose, testRun } };

		m_tasks.insert(std::pair<int, std::shared_ptr<RepRunner>>( taskId, runner));
		runner->AsyncRun();
	}
}

void CTaskListView::OnTaskDelete()
{
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		int item = GetListCtrl().GetNextSelectedItem(pos);

		if (AfxMessageBox(IDS_DELETE_CONFIRMATION, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		{
			Database::Table tb{ theApp.GetDB(), TASKS_TABLE };
			StringT condition = _T("TaskID=") + ToStringT(GetListCtrl().GetItemData(item));
			try
			{
				tb.Delete(condition);
				GetListCtrl().DeleteItem(item);
			}
			catch (Database::Exception e)
			{
				CString str(e.what());
				AfxMessageBox(str, MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}
}

struct tagEditColumnDef
{
	LPCTSTR colDBName;
	Database::PropertyType colDBType;
} EditColumnDef[] =
{
	{ TASKS_COL_TASKID, Database::PT_INT64 },
	{ TASKS_COL_NAME, Database::PT_TEXT },
	{ TASKS_COL_SOURCE, Database::PT_TEXT },
	{ TASKS_COL_DESTINATION, Database::PT_TEXT },
	{ TASKS_COL_FLASGS, Database::PT_INT },
	{ TASKS_COL_FILTERS, Database::PT_TEXT },
	{ TASKS_COL_DESTFOLDERFMT, Database::PT_TEXT }
};

void CTaskListView::OnTaskEdit()
{
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		int item = GetListCtrl().GetNextSelectedItem(pos);
		int taskId = static_cast<int>(GetListCtrl().GetItemData(item));

		Database::Table tb{ theApp.GetDB(), TASKS_TABLE };
		StringT condition = _T("TaskID=") + ToStringT(taskId);

		Database::PropertyList props;
		for (int i = 0; i < (sizeof(EditColumnDef) / sizeof(EditColumnDef[0])); ++i)
			props.push_back(Database::Property(EditColumnDef[i].colDBName, EditColumnDef[i].colDBType));

		Database::RecordsetPtr rs = tb.Select(props, condition);
		if (rs->Step())
		{
			CNewTaskDialog dlg(props, this);
			if (dlg.DoModal() == IDOK)
			{
				GetListCtrl().SetItemText(item, 0, dlg.GetNewTaskName());

				CMainFrame* pMain = static_cast<CMainFrame*>(theApp.GetMainWnd());
				pMain->Refresh(taskId, true);
			}
		}
	}
}

void CTaskListView::OnUpdateTaskEdit(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() > 0);
}

void CTaskListView::OnDestroy()
{
	// save column width
	SaveSettings();
	CListView::OnDestroy();
}

LRESULT CTaskListView::OnTaskDone(WPARAM wParam, LPARAM lParam)
{
	int item = static_cast<int>(wParam);
	int taskId = static_cast<int>(lParam);

	std::lock_guard<std::mutex> lock{ m_tasksLock };
	m_tasks.erase(taskId);

	// update row...
	Database::Table tb{ theApp.GetDB(), TASKS_TABLE };
	StringT condition = _T("TaskID=") + ToStringT(taskId);

	Database::PropertyList props;
	for (int i = 0; i < (sizeof(ColumnDef) / sizeof(ColumnDef[0])); ++i)
		props.push_back(Database::Property(ColumnDef[i].colDBName, ColumnDef[i].colDBType));

	Database::RecordsetPtr rs = tb.Select(props, condition);
	if (rs->Step())
	{
		if (item != -1)
		{
			GetListCtrl().SetItemText(item, LIST_COL_LAST_RUN, String::UTCTimeToLocalTime(rs->GetColumnStr(LIST_COL_LAST_RUN)).c_str());
			GetListCtrl().SetItemText(item, LIST_COL_LAST_SUCCESSFUL_RUN, String::UTCTimeToLocalTime(rs->GetColumnStr(LIST_COL_LAST_SUCCESSFUL_RUN)).c_str());
			GetListCtrl().SetItemText(item, LIST_COL_STATUS, rs->GetColumnStr(LIST_COL_STATUS).c_str());
		}
	}
	return 0;
}

void CTaskListView::LoadSettings()
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_TASKVIEW, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		for (int i = LIST_COL_NAME; i < MAX_UI_LIST_COLS; ++i)
		{
			StringT valueName = STR_TASKLIST_CW + ToStringT(i);
			DWORD value = static_cast<DWORD>(ColumnDef[i].width);
			DWORD size = static_cast<DWORD>(sizeof(DWORD));
			if (RegQueryValueEx(hKey, valueName.c_str(), 0, NULL, LPBYTE(&value), &size) == ERROR_SUCCESS)
				ColumnDef[i].width = static_cast<int>(value);
		}
		RegCloseKey(hKey);
	}
}

void CTaskListView::SaveSettings()
{
	bool columnChanged = false;

	for (int i = LIST_COL_NAME; i < MAX_UI_LIST_COLS; ++i)
	{
		int w = GetListCtrl().GetColumnWidth(i);
		if (w != ColumnDef[i].width)
		{
			ColumnDef[i].width = w;
			columnChanged = true;
		}
	}
	if (columnChanged)
	{
		DWORD disp = 0;
		HKEY hKey = NULL;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_TASKVIEW, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &disp) == ERROR_SUCCESS)
		{
			for (int i = LIST_COL_NAME; i < MAX_UI_LIST_COLS; ++i)
			{
				StringT valueName = STR_TASKLIST_CW + ToStringT(i);
				DWORD value = static_cast<DWORD>(ColumnDef[i].width);
				RegSetValueEx(hKey, valueName.c_str(), 0, REG_DWORD, LPBYTE(&value), sizeof(DWORD));
			}
			RegCloseKey(hKey);
		}
	}
}

void CTaskListView::EventCallback(CTaskListView* _This, int taskId, RunnerState state, LPCTSTR message)
{
	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	info.lParam = taskId;

	int index = _This->GetListCtrl().FindItem(&info);

	if (message && (index != -1))
		_This->GetListCtrl().SetItemText(index, LIST_COL_STATUS, message);

	if (state == RunnerState::STOP)
		_This->PostMessage(WM_TASK_DONE, static_cast<WPARAM>(index), static_cast<LPARAM>(taskId));
}

bool CTaskListView::IsTaskRunning(int taskId)
{
	// no lock find
	auto it = m_tasks.find(taskId);
	if (it != m_tasks.end())
	{
		return it->second->IsRunning();
	}
	return false;
}


void CTaskListView::OnTaskStop()
{
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		int item = GetListCtrl().GetNextSelectedItem(pos);

		if (AfxMessageBox(IDS_STOP_CONFIRMATION, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		{
			int taskId = static_cast<int>(GetListCtrl().GetItemData(item));
			std::lock_guard<std::mutex> lock{ m_tasksLock };
			auto it = m_tasks.find(taskId);
			if (it != m_tasks.end())
				it->second->Abort();
		}
	}
}

BOOL CTaskListView::IsSelectedTaskRunning()
{
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if (pos)
	{
		int item = GetListCtrl().GetNextSelectedItem(pos);
		int taskId = static_cast<int>(GetListCtrl().GetItemData(item));
		if (IsTaskRunning(taskId))
			return TRUE;
	}
	return FALSE;
}


void CTaskListView::OnUpdateTaskStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsSelectedTaskRunning());
}
