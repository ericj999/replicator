
// ReplicatorView.h : interface of the CReplicatorView class
//

#pragma once

#include "resource.h"
#include "afxcmn.h"

#include "GeneralPage.h"
#include "HistoryPage.h"

class CReplicatorView : public CFormView
{
protected: // create from serialization only
	CReplicatorView();
	DECLARE_DYNCREATE(CReplicatorView)

public:
	enum{ IDD = IDD_EASYDUPL_FORM };

// Attributes
public:
	CReplicatorDoc* GetDocument() const;

// Operations
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
	virtual ~CReplicatorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
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

#ifndef _DEBUG  // debug version in ReplicatorView.cpp
inline CReplicatorDoc* CReplicatorView::GetDocument() const
{ return reinterpret_cast<CReplicatorDoc*>(m_pDocument); }
#endif

