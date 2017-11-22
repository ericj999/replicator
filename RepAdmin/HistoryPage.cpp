// HistoryPage.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "HistoryPage.h"
#include "afxdialogex.h"


// CHistoryPage dialog

IMPLEMENT_DYNAMIC(CHistoryPage, CDialogEx)

CHistoryPage::CHistoryPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHistoryPage::IDD, pParent)
{

}

CHistoryPage::~CHistoryPage()
{
}

void CHistoryPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHistoryPage, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CHistoryPage message handlers


void CHistoryPage::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CWnd* pwnd = GetDlgItem(IDC_HISTORY_LIST);
	if (pwnd)
	{

		CRect rect;

		pwnd->GetWindowRect(&rect);
		ScreenToClient(&rect);

		pwnd->SetWindowPos(NULL, 0, 0, cx - (rect.left * 2), cy - (rect.top * 2), SWP_NOREPOSITION | SWP_NOZORDER);
	}
}
