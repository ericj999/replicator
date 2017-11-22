// SettingsPage.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "SettingsPage.h"
#include "afxdialogex.h"


// CSettingsPage dialog

IMPLEMENT_DYNAMIC(CSettingsPage, CDialogEx)

CSettingsPage::CSettingsPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSettingsPage::IDD, pParent)
{

}

CSettingsPage::~CSettingsPage()
{
}

void CSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSettingsPage, CDialogEx)
END_MESSAGE_MAP()


// CSettingsPage message handlers
