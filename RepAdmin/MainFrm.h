
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "WaitDialog.h"

class CMainFrame : public CFrameWndEx
{
public:
	CMainFrame();
	
protected: // create from serialization only
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:

// Operations
public:
	void Refresh(int taskID, DWORD refresh, bool force = false);
	void LoadSettings();
	void SaveSettings();

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//	static void ServerCallback(void* _This, const unsigned char* data, size_t size);

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;

	WaitDialog* m_waitDialog;
	INT_PTR m_timerId;
	int m_waitExitCounter;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTaskDelete();
	afx_msg void OnTaskNew();
	afx_msg void OnTaskRun();
	afx_msg void OnUpdateTaskRun(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTaskDelete(CCmdUI *pCmdUI);
	afx_msg void OnTaskEdit();
	afx_msg void OnUpdateTaskEdit(CCmdUI *pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnTaskStop();
	afx_msg void OnUpdateTaskStop(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnToolsSettings();
	afx_msg void OnHelpViewhelp();
};


