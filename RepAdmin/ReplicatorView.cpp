
// ReplicatorView.cpp : implementation of the CReplicatorView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Replicator.h"
#endif

#include "ReplicatorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum
{
	TAB_GENERAL = 0,
	TAB_HISTORY,
	MAX_TABS
};

int idsTabs[MAX_TABS] = {IDS_TAB_GENERAL, IDS_TAB_HISTORY};

// CReplicatorView

IMPLEMENT_DYNCREATE(CReplicatorView, CFormView)

BEGIN_MESSAGE_MAP(CReplicatorView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_TASK_DETAIL, &CReplicatorView::OnSelchangeTabTaskDetail)
END_MESSAGE_MAP()

// CReplicatorView construction/destruction

CReplicatorView::CReplicatorView()
	: CFormView(CReplicatorView::IDD), m_controlsCreated(false)
{
	// TODO: add construction code here

}

CReplicatorView::~CReplicatorView()
{
}

void CReplicatorView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_TASK_DETAIL, m_tab);
}

BOOL CReplicatorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CReplicatorView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
//	EnableScrollBarCtrl(SB_BOTH, FALSE);
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	if (!m_controlsCreated)
	{
		// OnInitialUpdate is called twice, add check here before finding a way to prevent this
		m_controlsCreated = true;
		for (int i = TAB_GENERAL; i < MAX_TABS; ++i)
		{
			CString str;
			str.LoadString(idsTabs[i]);
			m_tab.InsertItem(i, str);
		}
		m_pageGeneral.Create(IDD_GENERALPAGE, this);
		m_pageHistory.Create(IDD_HISTORYPAGE, this);
	}
	m_tab.SetCurSel(TAB_GENERAL);

	CWnd* ph = GetDlgItem(IDC_TAB_INNER);
	CRect rc;

	ph->GetWindowRect(&rc);
	ScreenToClient(&rc);

	m_pageGeneral.SetWindowPos(NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	m_pageHistory.SetWindowPos(NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	if (IsAppThemed())
	{
		EnableThemeDialogTexture(m_pageGeneral.GetSafeHwnd(), ETDT_ENABLETAB);
		EnableThemeDialogTexture(m_pageHistory.GetSafeHwnd(), ETDT_ENABLETAB);
	}
	UpdatePages();
}

void CReplicatorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
//	ClientToScreen(&point);
//	OnContextMenu(this, point);
}

void CReplicatorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
//	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CReplicatorView diagnostics

#ifdef _DEBUG
void CReplicatorView::AssertValid() const
{
	CFormView::AssertValid();
}

void CReplicatorView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


// CReplicatorView message handlers


void CReplicatorView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (m_tab.GetSafeHwnd())
	{
		CRect rect;

		GetClientRect(&rect);
		m_tab.MoveWindow(&rect, TRUE);
		m_tab.AdjustRect(FALSE, &rect);

		CRect rc;
		CWnd* ph = GetDlgItem(IDC_TAB_INNER);
		ph->GetWindowRect(&rc);

		ClientToScreen(&rect);
		int cx = rc.left - rect.left;

		rect.DeflateRect(cx, cx);

		if (m_pageGeneral.GetSafeHwnd())
			m_pageGeneral.SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);

		if (m_pageHistory.GetSafeHwnd())
			m_pageHistory.SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	}
}

void CReplicatorView::UpdatePages()
{
	int page = m_tab.GetCurSel();

	BOOL enableGeneral = (page == TAB_GENERAL);
	BOOL enableHistory = (page == TAB_HISTORY);

	m_pageGeneral.EnableWindow(enableGeneral);
	m_pageGeneral.ShowWindow(enableGeneral ? SW_SHOW : SW_HIDE);
	m_pageHistory.EnableWindow(enableHistory);
	m_pageHistory.ShowWindow(enableHistory ? SW_SHOW : SW_HIDE);
}


void CReplicatorView::OnSelchangeTabTaskDetail(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdatePages();
	*pResult = 0;
}

void CReplicatorView::Refresh(int taskID, DWORD refresh, bool force)
{
	if(refresh & REFRESH_GENERAL) m_pageGeneral.Refresh(taskID, force);
	if(refresh & REFRESH_HISTORY) m_pageHistory.Refresh(taskID, force);
}
