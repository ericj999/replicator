// AdvancedOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvancedOptionsDlg.h"
#include "afxdialogex.h"
#include "DBDef.h"
#include "resource.h"
#include "FolderFormatDialog.h"
#include "DatePathFormatter.h"

// UpdateOptions selections
enum
{
	UpdateOptionUpdateNewer = 0,
	UpdateOptionKeepBoth,
	UpdateOptionOverwrite,
	UpdateOptionDoNothing,
	UpdateOptionMax
};

int UpdateOptionID[UpdateOptionMax] =
{
	IDS_UPDATE_ACTION_NEWER,
	IDS_UPDATE_ACTION_KEEP_BOTH,
	IDS_UPDATE_ACTION_OVERWRITE,
	IDS_UPDATE_ACTION_DO_NOTHING,
};

// Sync action
enum
{
	SyncOptionKeep = 0,
	SyncOptionRemove,
	SyncOptionMax
};

int SyncOptionID[SyncOptionMax] =
{
	IDS_SYNC_ACTION_KEEP,
	IDS_SYNC_ACTION_REMOVE
};


// CAdvancedOptionsDlg dialog

IMPLEMENT_DYNAMIC(CAdvancedOptionsDlg, CDialogEx)

CAdvancedOptionsDlg::CAdvancedOptionsDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CAdvancedOptionsDlg::IDD, pParent), m_flags{ 0 }, m_options{ 0 }
{

}

CAdvancedOptionsDlg::~CAdvancedOptionsDlg()
{
}

void CAdvancedOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AO_UPDATE_OPTION, m_cbUpdateOptions);
}


BEGIN_MESSAGE_MAP(CAdvancedOptionsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_NEW_TASK_ALL_FILES, &CAdvancedOptionsDlg::OnBnClickedNewTaskAllFiles)
	ON_BN_CLICKED(IDC_NEW_TASK_INCLUSION, &CAdvancedOptionsDlg::OnBnClickedNewTaskInclusion)
	ON_BN_CLICKED(IDC_NEW_TASK_EXCLUSION, &CAdvancedOptionsDlg::OnBnClickedNewTaskExclusion)
	ON_BN_CLICKED(IDOK, &CAdvancedOptionsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_AO_EDIT_FORMAT, &CAdvancedOptionsDlg::OnBnClickedAoEditFormat)
	ON_BN_CLICKED(IDC_DESTINATION_GROUP, &CAdvancedOptionsDlg::OnBnClickedDestinationGroup)
	ON_BN_CLICKED(IDC_DESTINATION_SAME_FROM_SELECT, &CAdvancedOptionsDlg::OnBnClickedDestinationSameFromSelect)
	ON_BN_CLICKED(IDC_DESTINATION_SAME_FROM_ROOT, &CAdvancedOptionsDlg::OnBnClickedDestinationSameFromRoot)
END_MESSAGE_MAP()


// CAdvancedOptionsDlg message handlers
BOOL CAdvancedOptionsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// filters
	GetDlgItem(IDC_AO_FILTERS)->SetWindowText(m_filters);
	if (m_flags & TASKS_FLAGS_INCLUDE_FILTERS)
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUSION));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);

		GetDlgItem(IDC_AO_FILTERS)->EnableWindow();
	}
	else if (m_flags & TASKS_FLAGS_EXCLUDE_FILTERS)
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_EXCLUSION));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);

		GetDlgItem(IDC_AO_FILTERS)->EnableWindow();
	}
	else
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_ALL_FILES));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_AO_FILTERS)->EnableWindow(FALSE);
	}

	// folder options
	if (m_flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_DESTINATION_GROUP));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);
	}
	else if (m_flags & TASKS_FLAGS_DEST_START_FROM_ROOT)
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_DESTINATION_SAME_FROM_ROOT));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);
	}
	else
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_DESTINATION_SAME_FROM_SELECT));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);
	}

	UpdateFolderOption(m_flags);

	GetDlgItem(IDC_AO_DESTINATION_FORMAT)->SetWindowText(m_destinationFolderFormat);

	// update options
	CString str;
	for (int index = 0; index < UpdateOptionMax; ++index)
	{
		str.LoadString(UpdateOptionID[index]);
		m_cbUpdateOptions.InsertString(index, str);
	}

	if (m_flags & TASKS_FLAGS_UPDATE_KEEP_BOTH)
		m_cbUpdateOptions.SetCurSel(UpdateOptionKeepBoth);
	else if (m_flags & TASKS_FLAGS_UPDATE_DO_NOTHING)
		m_cbUpdateOptions.SetCurSel(UpdateOptionDoNothing);
	else if (m_flags & TASKS_FLAGS_UPDATE_OVERWRITE)
		m_cbUpdateOptions.SetCurSel(UpdateOptionOverwrite);
	else
		m_cbUpdateOptions.SetCurSel(UpdateOptionUpdateNewer);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAdvancedOptionsDlg::OnBnClickedNewTaskAllFiles()
{
	CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_ALL_FILES));
	if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
		GetDlgItem(IDC_AO_FILTERS)->EnableWindow(FALSE);
}


void CAdvancedOptionsDlg::OnBnClickedNewTaskInclusion()
{
	CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUSION));
	if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
		GetDlgItem(IDC_AO_FILTERS)->EnableWindow();
}


void CAdvancedOptionsDlg::OnBnClickedNewTaskExclusion()
{
	CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_EXCLUSION));
	if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
		GetDlgItem(IDC_AO_FILTERS)->EnableWindow();
}

void CAdvancedOptionsDlg::OnBnClickedOk()
{
	m_flags = 0;
	CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUSION));
	if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
		m_flags |= TASKS_FLAGS_INCLUDE_FILTERS;
	else
	{
		pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_EXCLUSION));
		if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
			m_flags |= TASKS_FLAGS_EXCLUDE_FILTERS;
	}
	if ((m_flags & TASKS_FLAGS_INCLUDE_FILTERS) || (m_flags & TASKS_FLAGS_EXCLUDE_FILTERS))
	{
		GetDlgItem(IDC_AO_FILTERS)->GetWindowText(m_filters);
		if (m_filters.IsEmpty())
		{
			AfxMessageBox(IDS_FILTER_IS_EMPTY);
			return;
		}
	}
	else
	{
		m_filters.Empty();
	}

	pBtn = static_cast<CButton*>(GetDlgItem(IDC_DESTINATION_GROUP));
	if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
	{
		m_flags |= TASKS_FLAGS_DEST_GROUP_BY_DATE;
		GetDlgItem(IDC_AO_DESTINATION_FORMAT)->GetWindowText(m_destinationFolderFormat);
		if (m_destinationFolderFormat.IsEmpty())
		{
			AfxMessageBox(IDS_FOLDER_FORMAT_IS_EMPTY);
			return;
		}
		if (!(m_flags & TASKS_FLAGS_UPDATE_KEEP_BOTH))
		{
			if (AfxMessageBox(IDS_WARN_OVERWRITE, MB_YESNO) != IDYES)
				return;
		}

		int sel = m_cbUpdateOptions.GetCurSel();
		if (sel == UpdateOptionKeepBoth)
			m_flags |= TASKS_FLAGS_UPDATE_KEEP_BOTH;
		else if (sel == UpdateOptionOverwrite)
			m_flags |= TASKS_FLAGS_UPDATE_OVERWRITE;
		else if (sel == UpdateOptionDoNothing)
			m_flags |= TASKS_FLAGS_UPDATE_DO_NOTHING;
		else
			m_flags |= TASKS_FLAGS_UPDATE_NEWER;
	}
	else
	{
		m_destinationFolderFormat.Empty();
		pBtn = static_cast<CButton*>(GetDlgItem(IDC_DESTINATION_SAME_FROM_ROOT));
		if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
			m_flags |= TASKS_FLAGS_DEST_START_FROM_ROOT;
	}
	CDialogEx::OnOK();
}

void CAdvancedOptionsDlg::UpdateFolderOption(int flag)
{
	BOOL enable = FALSE;
	if (flag & TASKS_FLAGS_DEST_GROUP_BY_DATE)
		enable = TRUE;

	GetDlgItem(IDC_AO_EDIT_FORMAT)->EnableWindow(enable);
	GetDlgItem(IDC_AO_DESTINATION_FORMAT)->EnableWindow(enable);
	GetDlgItem(IDC_AO_DESTINATION_FORMAT_EXAMPLE)->EnableWindow(enable);
	GetDlgItem(IDC_ACTION_TEXT)->EnableWindow(enable);
	GetDlgItem(IDC_AO_UPDATE_OPTION)->EnableWindow(enable);
}


void CAdvancedOptionsDlg::OnBnClickedAoEditFormat()
{
	FolderFormatDialog ffd{ this };

	CString str;
	GetDlgItem(IDC_AO_DESTINATION_FORMAT)->GetWindowText(str);
	ffd.set_format(str);
	if (ffd.DoModal() == IDOK)
	{
		GetDlgItem(IDC_AO_DESTINATION_FORMAT)->SetWindowText(ffd.get_format());
	}
}

void CAdvancedOptionsDlg::OnBnClickedDestinationGroup()
{
	UpdateFolderOption(TASKS_FLAGS_DEST_GROUP_BY_DATE);
}

void CAdvancedOptionsDlg::OnBnClickedDestinationSameFromSelect()
{
	UpdateFolderOption(0);
}

void CAdvancedOptionsDlg::OnBnClickedDestinationSameFromRoot()
{
	UpdateFolderOption(TASKS_FLAGS_DEST_START_FROM_ROOT);
}
