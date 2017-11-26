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
	m_historyDays{ DEFAULT_HISTORY_DAYS }, m_flags{ 0 }
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
}


BEGIN_MESSAGE_MAP(SettingsDialog, CDialogEx)
	ON_BN_CLICKED(IDC_REMOVE_OLDER_HISTORY, &SettingsDialog::OnBnClickedRemoveOlderHistory)
	ON_BN_CLICKED(IDOK, &SettingsDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// SettingsDialog message handlers


BOOL SettingsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_flags & SETTINGFLAGS_DELETE_OLDER_HISTORY)
	{
		CheckDlgButton(IDC_REMOVE_OLDER_HISTORY, BST_CHECKED);
	}
	OnBnClickedRemoveOlderHistory();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void SettingsDialog::OnBnClickedRemoveOlderHistory()
{
	CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_REMOVE_OLDER_HISTORY));
	GetDlgItem(IDC_EDIT_HISTORY_DAYS)->EnableWindow((pBtn && (pBtn->GetCheck() == BST_CHECKED)) ? TRUE : FALSE);
}


void SettingsDialog::OnBnClickedOk()
{
	if (IsDlgButtonChecked(IDC_REMOVE_OLDER_HISTORY))
		m_flags |= SETTINGFLAGS_DELETE_OLDER_HISTORY;
	else
		m_flags &= ~SETTINGFLAGS_DELETE_OLDER_HISTORY;

	CDialogEx::OnOK();
}
