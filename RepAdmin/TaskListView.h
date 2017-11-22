
// LeftView.h : interface of the CTaskListView class
//
#pragma once

#include <map>
#include <thread>
#include <mutex>

#include "Recordset.h"
#include "RepRunner.h"


class CReplicatorDoc;

class CTaskListView : public CListView
{
protected: // create from serialization only
	CTaskListView();
	DECLARE_DYNCREATE(CTaskListView)

// Attributes
public:
	CReplicatorDoc* GetDocument();

// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CTaskListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// member functions
	bool IsBusy() { return m_tasks.size();  }
	BOOL IsSelectedTaskRunning();

	// callback
	static void EventCallback(CTaskListView* _This, int taskId, RunnerState state, LPCTSTR message);

protected:
	std::map<int, std::shared_ptr<RepRunner>> m_tasks;
	std::mutex m_tasksLock;

	bool IsTaskRunning(int taskId);

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
};

#ifndef _DEBUG  // debug version in LeftView.cpp
inline CReplicatorDoc* CTaskListView::GetDocument()
{ return reinterpret_cast<CReplicatorDoc*>(m_pDocument); }
#endif

