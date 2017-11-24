// HistoryPage.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "HistoryPage.h"
#include "afxdialogex.h"
#include "resource.h"

#define REGSTR_KEY_CONFIG_HISTORY_VIEW _T("SOFTWARE\\Replicator\\Config\\HistoryView")
#define STR_HISTORYLIST_CW _T("HistoryListCW")

enum
{
	COL_START_TIME = 0,
	COL_END_TIME,
	COL_RESULT,
	MAX_COLS
};

struct tagColDef
{
	int ids;
	int width;
	LPCTSTR colName;
	Database::PropertyType colType;
}  columns[MAX_COLS] =
{
	{ IDS_HP_COL_START_TIME, 120, HISTORY_COL_START_TIME, Database::PT_TEXT },
	{ IDS_HP_COL_END_TIME, 120, HISTORY_COL_END_TIME, Database::PT_TEXT },
	{ IDS_HP_COL_RESULT, 600, HISTORY_COL_RESULT, Database::PT_TEXT }
};

// CHistoryPage dialog

IMPLEMENT_DYNAMIC(CHistoryPage, CDialogEx)

CHistoryPage::CHistoryPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHistoryPage::IDD, pParent),
	m_CurrentTask(-1), m_table(NULL), m_totalRows(0), m_cacheOffset(0)
{
	m_table = new Database::Table(theApp.GetDB(), HISTORY_TABLE);
}

CHistoryPage::~CHistoryPage()
{
	if (m_table)
		delete m_table;
}

void CHistoryPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HISTORY_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CHistoryPage, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_HISTORY_LIST, &CHistoryPage::OnGetdispinfoHistoryList)
	ON_NOTIFY(LVN_ODCACHEHINT, IDC_HISTORY_LIST, &CHistoryPage::OnOdcachehintHistoryList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CHistoryPage message handlers
void CHistoryPage::Refresh(int newTask, bool force)
{
	if ((newTask != m_CurrentTask) || (force && (newTask == m_CurrentTask)))
	{
		m_totalRows = 0;

		if (m_table)
		{
			int c = m_table->GetCount(Database::Property(HISTORY_COL_TASKID, newTask));
			if (c > 0)
				m_totalRows = c;
		}
		// change the list count
		m_list.DeleteAllItems();
		m_list.SetItemCountEx(m_totalRows);
		m_CurrentTask = newTask;
	}
}

void CHistoryPage::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_list.GetSafeHwnd())
	{
		CRect rect, ctrlRect;

		GetWindowRect(&rect);
		m_list.GetWindowRect(&ctrlRect);

		int dx = ctrlRect.left - rect.left;
		int dy = ctrlRect.top - rect.top;

		m_list.MoveWindow(0, 0, cx - (dx * 2), cy - (dy * 2));
	}
}


BOOL CHistoryPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LoadSettings();
	for (int i = 0; i < MAX_COLS; ++i)
	{
		CString str;
		str.LoadString(columns[i].ids);
		m_list.InsertColumn(i, str, LVCFMT_LEFT, columns[i].width);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CHistoryPage::OnGetdispinfoHistoryList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem = &(pDispInfo)->item;

	int iItem = pItem->iItem;

	if ((iItem >= m_cacheOffset) && (iItem < m_cacheOffset + m_cacheItems.size()))
	{
		int offset = iItem - m_cacheOffset;
		if (pItem->mask & LVIF_TEXT) //valid text buffer?
		{
			switch (pItem->iSubItem)
			{
			case COL_START_TIME:
				_tcscpy_s(pItem->pszText, pItem->cchTextMax, m_cacheItems[offset].startTime.c_str());
				break;
			case COL_END_TIME:
				_tcscpy_s(pItem->pszText, pItem->cchTextMax, m_cacheItems[offset].endTime.c_str());
				break;
			case COL_RESULT:
				_tcscpy_s(pItem->pszText, pItem->cchTextMax, m_cacheItems[offset].result.c_str());
				break;
			}
		}
	}
	*pResult = 0;
}


void CHistoryPage::OnOdcachehintHistoryList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCACHEHINT pCacheHint = reinterpret_cast<LPNMLVCACHEHINT>(pNMHDR);

	// we will cache double of hint data
	int requestCacheCount = pCacheHint->iTo - pCacheHint->iFrom + 1;

	if ((requestCacheCount > 0) && (m_CurrentTask >= 0) && m_table)
	{
		int low = pCacheHint->iFrom - (requestCacheCount / 2);
		int high = pCacheHint->iTo + (requestCacheCount / 2);

		if (low < 0) low = 0;
		if (high >= m_totalRows) high = m_totalRows - 1;

		m_cacheOffset = low;

		int selectCount = high - low + 1;

		StringT condition = HISTORY_COL_TASKID;
		condition += _T("=") + ToStringT(m_CurrentTask)
			+ _T(" ORDER BY rid DESC LIMIT ") + ToStringT(low) + _T(",") + ToStringT(selectCount);

		Database::PropertyList props;
		for (int i = 0; i < MAX_COLS; ++i)
			props.push_back(Database::Property(columns[i].colName, columns[i].colType));

		m_cacheItems.clear();
		Database::RecordsetPtr rs = m_table->Select(props, condition);
		// Update the cache with the recommended range.
		while (rs->Step())
		{
			ListItem li;

			li.startTime = String::UTCTimeToLocalTime(rs->GetColumnStr(COL_START_TIME));
			li.endTime = String::UTCTimeToLocalTime(rs->GetColumnStr(COL_END_TIME));
			li.result = rs->GetColumnStr(COL_RESULT);
			
			m_cacheItems.push_back(std::move(li));
		}
	}
	*pResult = 0;
}

void CHistoryPage::LoadSettings()
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_HISTORY_VIEW, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		for (int i = 0; i < MAX_COLS; ++i)
		{
			StringT valueName = STR_HISTORYLIST_CW + ToStringT(i);
			DWORD value = static_cast<DWORD>(columns[i].width);
			DWORD size = static_cast<DWORD>(sizeof(DWORD));
			if (RegQueryValueEx(hKey, valueName.c_str(), 0, NULL, LPBYTE(&value), &size) == ERROR_SUCCESS)
				columns[i].width = static_cast<int>(value);
		}
		RegCloseKey(hKey);
	}
}

void CHistoryPage::SaveSettings()
{
	bool columnChanged = false;

	for (int i = 0; i < MAX_COLS; ++i)
	{
		int w = m_list.GetColumnWidth(i);
		if (w != columns[i].width)
		{
			columns[i].width = w;
			columnChanged = true;
		}
	}
	if (columnChanged)
	{
		DWORD disp = 0;
		HKEY hKey = NULL;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_HISTORY_VIEW, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &disp) == ERROR_SUCCESS)
		{
			for (int i = 0; i < MAX_COLS; ++i)
			{
				StringT valueName = STR_HISTORYLIST_CW + ToStringT(i);
				DWORD value = static_cast<DWORD>(columns[i].width);
				RegSetValueEx(hKey, valueName.c_str(), 0, REG_DWORD, LPBYTE(&value), sizeof(DWORD));
			}
			RegCloseKey(hKey);
		}
	}
}

void CHistoryPage::OnDestroy()
{
	SaveSettings();
	CDialogEx::OnDestroy();
}
