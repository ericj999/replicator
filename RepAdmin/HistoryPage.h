#pragma once
#include "afxcmn.h"
#include "Table.h"

#include <vector>

struct ListItem
{
	StringT startTime;
	StringT endTime;
	StringT result;
};

// CHistoryPage dialog

class CHistoryPage : public CDialogEx
{
	DECLARE_DYNAMIC(CHistoryPage)

public:
	CHistoryPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHistoryPage();

	void Refresh(int newTask, bool force);

// Dialog Data
	enum { IDD = IDD_HISTORYPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	int m_CurrentTask;
	Database::Table* m_table;

	int m_totalRows;
	size_t m_cacheOffset;
	std::vector<ListItem> m_cacheItems;

	void LoadSettings();
	void SaveSettings();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	CListCtrl m_list;
	afx_msg void OnGetdispinfoHistoryList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOdcachehintHistoryList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
};
