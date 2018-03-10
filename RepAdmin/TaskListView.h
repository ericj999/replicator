
// LeftView.h : interface of the CTaskListView class
//
#pragma once

#include <map>
#include <thread>
#include <mutex>

#include "Recordset.h"
#include "RepRunner.h"


class CTaskListView : public CListView
{
	DECLARE_DYNCREATE(CTaskListView)

protected: // create from serialization only
	CTaskListView();
	virtual ~CTaskListView();

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// member functions
	bool IsBusy() { return m_tasks.size();  }
	BOOL IsSelectedTaskRunning();
	void StopAllTasks(bool stopCallback = false);

	// callback
	static void EventCallback(CTaskListView* _This, int taskId, RunnerState state, LPCTSTR message);

protected:
	std::map<int, std::shared_ptr<RepRunner>> m_tasks;
	std::mutex m_tasksLock;

	bool IsTaskRunning(int taskId);
	void RunTask(int item);

	int GetSelectedTask();
	void AddNewTask(LPCTSTR taskName);
	int InsertItem(Database::RecordsetPtr rs);
	void LoadSettings();
	void SaveSettings();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateTaskDelete(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTaskNew(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTaskRun(CCmdUI *pCmdUI);
	afx_msg void OnTaskNew();
	afx_msg void OnTaskRun();
	afx_msg void OnTaskDelete();
	afx_msg void OnTaskEdit();
	afx_msg void OnUpdateTaskEdit(CCmdUI *pCmdUI);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTaskDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTaskStop();
	afx_msg void OnUpdateTaskStop(CCmdUI *pCmdUI);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
};

