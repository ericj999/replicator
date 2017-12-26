// GeneralPage.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "GeneralPage.h"
#include "afxdialogex.h"

#include "DBDef.h"
#include "Table.h"
#include "StringT.h"

enum
{
	COL_Name = 0,
	COL_CreatedTime,
	COL_Source,
	COL_Destination,
	COL_Flags,
	COL_Filters,
	MAX_COLS
};
// CGeneralPage dialog
struct tagInfoColumnDef
{
	LPCTSTR colDBName;
	Database::PropertyType colDBType;
} ColumnDef[] =
{
	{ TASKS_COL_NAME, Database::PT_TEXT },
	{ TASKS_COL_CREATEDTIME, Database::PT_TEXT },
	{ TASKS_COL_SOURCE, Database::PT_TEXT },
	{ TASKS_COL_DESTINATION, Database::PT_TEXT },
	{ TASKS_COL_FLASGS, Database::PT_INT },
	{ TASKS_COL_FILTERS, Database::PT_TEXT },
	{ TASKS_COL_DESTFOLDERFMT, Database::PT_TEXT },
};

IMPLEMENT_DYNAMIC(CGeneralPage, CDialogEx)

CGeneralPage::CGeneralPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGeneralPage::IDD, pParent),
	m_CurrentTask(-1)
{

}

CGeneralPage::~CGeneralPage()
{
}

void CGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGeneralPage, CDialogEx)
END_MESSAGE_MAP()


// CGeneralPage message handlers
void CGeneralPage::Refresh(int newTask, bool force)
{
	if ((newTask != m_CurrentTask) || (force && (newTask == m_CurrentTask)))
	{
		StringT name, taskId, createdTime, sourcePath, destPath, repCondition;

		if (newTask > 0)
		{
			Database::Table tb{ theApp.GetDB(), TASKS_TABLE };

			StringT condition = TASKS_COL_TASKID;
			condition += _T("=") + ToStringT(newTask);

			Database::PropertyList props;
			for (int i = 0; i < MAX_COLS; ++i)
				props.push_back(Database::Property(ColumnDef[i].colDBName, ColumnDef[i].colDBType));

			Database::RecordsetPtr rs = tb.Select(props, condition);
			if (rs->Step())
			{
				CString s;

				condition.clear();
				int flags = rs->GetColumnInt(COL_Flags);
				if (flags & TASKS_FLAGS_INCLUDE_SUBDIR)
					s.LoadString(IDS_CHILD_FOLDER);

				condition = s + _T("\r\n");

				if (flags & TASKS_FLAGS_INCLUDE_FILTERS)
				{
					s.LoadString(IDS_FILES_MATCHING);
					s += rs->GetColumnStr(COL_Filters).c_str();
				}
				else if (flags & TASKS_FLAGS_EXCLUDE_FILTERS)
				{
					s.LoadString(IDS_FILES_NOT_MATCHING);
					s += rs->GetColumnStr(COL_Filters).c_str();
				}
				else
					s.LoadStringW(IDS_ALL_FILES);

				condition += s + _T("\r\n");

				name = rs->GetColumnStr(COL_Name);
				createdTime = String::UTCTimeToLocalTime(rs->GetColumnStr(COL_CreatedTime));
				sourcePath = rs->GetColumnStr(COL_Source);
				destPath = rs->GetColumnStr(COL_Destination);
				repCondition = condition;
			}
		}

		taskId = ToStringT(newTask);
		m_CurrentTask = newTask;

		SetDlgItemText(IDC_TASK_NAME, name.c_str());
		SetDlgItemText(IDC_TASK_ID, taskId.c_str());
		SetDlgItemText(IDC_CREATED_TIME, createdTime.c_str());
		SetDlgItemText(IDC_SOURCE_PATH, sourcePath.c_str());
		SetDlgItemText(IDC_DEST_PATH, destPath.c_str());
		SetDlgItemText(IDC_CONDITIONS, repCondition.c_str());
	}
}
