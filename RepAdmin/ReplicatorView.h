
// ReplicatorView.h : interface of the CReplicatorView class
//

#pragma once

#include "resource.h"
#include "afxcmn.h"

#include "GeneralPage.h"
#include "HistoryPage.h"

class CReplicatorView : public CFormView
{
	DECLARE_DYNCREATE(CReplicatorView)

protected: // create from serialization only
	CReplicatorView();
	virtual ~CReplicatorView();

public:
	enum{ IDD = IDD_EASYDUPL_FORM };

public:
	void Refresh(int taskID, DWORD refresh, bool force);
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool m_controlsCreated;
	CGeneralPage m_pageGeneral;
	CHistoryPage m_pageHistory;

	void UpdatePages();

// Generated message map functions
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tab;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTabTaskDetail(NMHDR *pNMHDR, LRESULT *pResult);
};
