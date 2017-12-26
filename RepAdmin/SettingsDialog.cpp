// SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "SettingsDialog.h"
#include "afxdialogex.h"


// SettingsDialog dialog

IMPLEMENT_DYNAMIC(SettingsDialog, CDialogEx)

SettingsDialog::SettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent),
	m_historyDays{ DEFAULT_HISTORY_DAYS }, m_testRun(FALSE)
{

}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HISTORY_DAYS, m_historyDays);
	DDV_MinMaxInt(pDX, m_historyDays, HISTORY_DAYS_MIN, HISTORY_DAYS_MAX);
	DDX_Check(pDX, IDC_TEST_RUN, m_testRun);
}


BEGIN_MESSAGE_MAP(SettingsDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &SettingsDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// SettingsDialog message handlers


BOOL SettingsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void SettingsDialog::OnBnClickedOk()
{
	CDialogEx::OnOK();
}
