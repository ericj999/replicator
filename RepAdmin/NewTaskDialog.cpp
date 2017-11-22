// NewTaskDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "NewTaskDialog.h"
#include "afxdialogex.h"
#include "StringT.h"
#include "Table.h"
#include "DBDef.h"

#include "AdvancedOptionsDlg.h"

// CNewTaskDialog dialog

IMPLEMENT_DYNAMIC(CNewTaskDialog, CDialogEx)

CNewTaskDialog::CNewTaskDialog(CWnd* pParent /*=NULL*/)
: CDialogEx(CNewTaskDialog::IDD, pParent), m_update{ false }, m_AdvnacedOptionsFlags{ TASKS_FLAGS_UPDATE_NEWER }, m_changed{ false }, m_ready{ false }
{

}

CNewTaskDialog::CNewTaskDialog(const Database::PropertyList& props, CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewTaskDialog::IDD, pParent), m_update{ true }, m_props(props), m_AdvnacedOptionsFlags{ TASKS_FLAGS_UPDATE_NEWER }, m_changed{ false }, m_ready{ false }
{

}


CNewTaskDialog::~CNewTaskDialog()
{
}

void CNewTaskDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEW_TASK_SOURCE, m_listSources);
	DDX_Control(pDX, IDC_NEW_TASK_NAME, m_editName);
	DDX_Control(pDX, IDC_NEW_TASK_DESTINATION, m_editDest);
}


BEGIN_MESSAGE_MAP(CNewTaskDialog, CDialogEx)
	ON_BN_CLICKED(IDC_NEW_TASK_ADD_SRC, &CNewTaskDialog::OnBnClickedNewtaskAddSrc)
	ON_BN_CLICKED(IDC_NEW_TASK_REMOVE_SRC, &CNewTaskDialog::OnBnClickedNewtaskRemoveSrc)
	ON_BN_CLICKED(IDC_NEW_TASK_BROWSE_DEST, &CNewTaskDialog::OnBnClickedNewtaskBrowseDest)
	ON_BN_CLICKED(IDOK, &CNewTaskDialog::OnBnClickedOk)
	ON_EN_CHANGE(IDC_NEW_TASK_NAME, &CNewTaskDialog::OnEnChange)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_NEW_TASK_SOURCE, &CNewTaskDialog::OnLVItemChanged)
	ON_EN_CHANGE(IDC_NEW_TASK_DESTINATION, &CNewTaskDialog::OnEnChangeNewTaskDestination)
	ON_BN_CLICKED(IDC_NEW_TASK_CREATE, &CNewTaskDialog::OnBnClickedNewTaskCreate)
	ON_BN_CLICKED(IDC_NEW_TASK_ADV_OPTIONS, &CNewTaskDialog::OnBnClickedNewTaskAdvOptions)
	ON_BN_CLICKED(IDC_NEW_TASK_INCLUDE_SUB, &CNewTaskDialog::OnBnClickedNewTaskIncludeSub)
END_MESSAGE_MAP()


// CNewTaskDialog message handlers


BOOL CNewTaskDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CRect rc;

	m_listSources.GetClientRect(&rc);
	m_listSources.InsertColumn(0, _T(""));
	m_listSources.SetColumnWidth(0, rc.Width() - GetSystemMetrics(SM_CXVSCROLL));

	UpdateControls();
	m_ready = true;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNewTaskDialog::UpdateControls()
{
	if (m_update)
	{
		CString str;

		str.LoadString(IDS_EDIT_TASK);
		SetWindowText(str);

		str.LoadString(IDS_UPDATE);
		SetDlgItemText(IDC_NEW_TASK_CREATE, str);

		const Database::Property& taskName = m_props.Find(TASKS_COL_NAME);
		if (!taskName.IsNULL()) m_editName.SetWindowText(taskName.m_str.c_str());

		const Database::Property& srcProp = m_props.Find(TASKS_COL_SOURCE);
		if (!srcProp.IsNULL())
		{
			std::vector<StringT> sources;

			String::Tokenize(srcProp.m_str, sources, STR_SRC_PATH_SEPARATOR);
			for (auto&& s : sources)
				m_listSources.InsertItem(m_listSources.GetItemCount(), s.c_str());
		}

		const Database::Property& dest = m_props.Find(TASKS_COL_DESTINATION);
		if (!dest.IsNULL()) m_editDest.SetWindowText(dest.m_str.c_str());

		const Database::Property& destFolderFormat = m_props.Find(TASKS_COL_DESTFOLDERFMT);
		if (!destFolderFormat.IsNULL()) 
			m_strDestinationFolderFormat = destFolderFormat.m_str.c_str();

		const Database::Property& flags = m_props.Find(TASKS_COL_FLASGS);
		if (flags.IsNULL())
		{
			CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUDE_SUB));
			if (pBtn) pBtn->SetCheck(BST_CHECKED);
		}
		else
		{
			CButton* pBtn = NULL;
			if (flags.m_i & TASK_INCLUDE_SUBDIR)
			{
				pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUDE_SUB));
				if (pBtn) pBtn->SetCheck(BST_CHECKED);
			}
			const Database::Property& filters = m_props.Find(TASKS_COL_FILTERS);
			if (!filters.IsNULL())
				m_strFilters = filters.m_str.c_str();

			m_AdvnacedOptionsFlags = flags.m_i & TASKS_FLAGS_ADV_OPT_MASKS;
		}
	}
	else
	{
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUDE_SUB));
		if (pBtn) pBtn->SetCheck(BST_CHECKED);
	}
}

void CNewTaskDialog::OnBnClickedNewtaskAddSrc()
{
	CFolderPickerDialog fpg(NULL, 0, this);
	if (fpg.DoModal() == IDOK)
	{
		m_listSources.InsertItem(m_listSources.GetItemCount(), fpg.GetPathName());
		setChanged();
		UpdateCreateButtons();
	}
}


void CNewTaskDialog::OnBnClickedNewtaskRemoveSrc()
{
	POSITION pos = m_listSources.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_listSources.GetNextSelectedItem(pos);
		m_listSources.DeleteItem(nItem);
		UpdateCreateButtons();
	}
}


void CNewTaskDialog::OnBnClickedNewtaskBrowseDest()
{
	CFolderPickerDialog fpg(NULL, 0, this);
	if (fpg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_NEW_TASK_DESTINATION, fpg.GetPathName());
		setChanged();
	}
}


void CNewTaskDialog::OnBnClickedOk()
{
}


void CNewTaskDialog::OnEnChange()
{
	setChanged();
	UpdateCreateButtons();
}


void CNewTaskDialog::OnLVItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	GetDlgItem(IDC_NEW_TASK_REMOVE_SRC)->EnableWindow((m_listSources.GetSelectedCount() > 0) ? TRUE : FALSE);
	*pResult = 0;
}


void CNewTaskDialog::OnEnChangeNewTaskDestination()
{
	setChanged();
	UpdateCreateButtons();
}

void CNewTaskDialog::UpdateCreateButtons()
{
	BOOL bEnable = (m_changed && (m_editName.GetWindowTextLength() > 0) && (m_editDest.GetWindowTextLength() > 0) && (m_listSources.GetItemCount() > 0)) ? TRUE : FALSE;
	GetDlgItem(IDC_NEW_TASK_CREATE)->EnableWindow(bEnable);
}

void CNewTaskDialog::OnBnClickedNewTaskCreate()
{
	try
	{
		StringT source;
		for (int index = 0; index < m_listSources.GetItemCount(); ++index)
		{
			if (index != 0) source += STR_SRC_PATH_SEPARATOR;
			source += m_listSources.GetItemText(index, 0);
		}

		CString strName, strDest;
		m_editName.GetWindowText(strName);
		m_editDest.GetWindowText(strDest);

		int flags = 0;
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUDE_SUB));
		if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
			flags |= TASK_INCLUDE_SUBDIR;

		flags |= m_AdvnacedOptionsFlags;

		Database::PropertyList propList;
		Database::Table table{ theApp.GetDB(), TASKS_TABLE };
		if (m_update)
		{
			if (m_props.Find(TASKS_COL_NAME).m_str.c_str() != strName)
				propList.push_back(Database::Property(TASKS_COL_NAME, strName));

			if (m_props.Find(TASKS_COL_SOURCE).m_str.c_str() != source)
				propList.push_back(Database::Property(TASKS_COL_SOURCE, source));

			if (m_props.Find(TASKS_COL_DESTINATION).m_str.c_str() != strDest)
				propList.push_back(Database::Property(TASKS_COL_DESTINATION, strDest));

			if (m_props.Find(TASKS_COL_DESTFOLDERFMT).m_str.c_str() != m_strDestinationFolderFormat)
				propList.push_back(Database::Property(TASKS_COL_DESTFOLDERFMT, m_strDestinationFolderFormat));

			if (m_props.Find(TASKS_COL_FLASGS).m_i != flags)
				propList.push_back(Database::Property(TASKS_COL_FLASGS, flags));

			if (m_props.Find(TASKS_COL_FILTERS).m_str.c_str() != m_strFilters)
				propList.push_back(Database::Property(TASKS_COL_FILTERS, m_strFilters));

			if (!propList.empty())
			{
				Database::Property taskID = m_props.Find(TASKS_COL_TASKID);
				StringT id = taskID.ToString();

				if (!id.empty())
				{
					StringT condition = TASKS_COL_TASKID;
					condition += _T("=") + id;

					table.Update(propList, condition);
				}
				else
				{
					AfxMessageBox(IDS_INVALID_TASKID, MB_OK | MB_ICONSTOP);
				}
			}
		}
		else
		{
			if (table.GetCount(Database::Property(TASKS_COL_NAME, strName)) > 0)
			{
				AfxMessageBox(IDS_ERROR_SAME_NAME, MB_OK | MB_ICONEXCLAMATION);
				return;
			}
			propList.push_back(Database::Property(TASKS_COL_NAME, strName));
			propList.push_back(Database::Property(TASKS_COL_SOURCE, source));
			propList.push_back(Database::Property(TASKS_COL_DESTINATION, strDest));
			propList.push_back(Database::Property(TASKS_COL_DESTFOLDERFMT, m_strDestinationFolderFormat));
			propList.push_back(Database::Property(TASKS_COL_FLASGS, flags));
			propList.push_back(Database::Property(TASKS_COL_FILTERS, m_strFilters));

			table.Insert(propList);
		}
		m_strNewTaskName = strName;
	}
	catch (Database::Exception e)
	{
		CString msg(e.what());
		AfxMessageBox(msg, MB_OK | MB_ICONEXCLAMATION);
	}
	OnOK();
}


void CNewTaskDialog::OnBnClickedNewTaskAdvOptions()
{
	CAdvancedOptionsDlg advDlg(this);

	advDlg.set_Flags(m_AdvnacedOptionsFlags);
	advDlg.set_Filters(m_strFilters);
	advDlg.set_DestinationFolderFormat(m_strDestinationFolderFormat);

	if (advDlg.DoModal() == IDOK)
	{
		setChanged();
		m_AdvnacedOptionsFlags = advDlg.get_Flags();
		m_strFilters = advDlg.get_Filters();
		m_strDestinationFolderFormat = advDlg.get_DestinationFolderFormat();
		UpdateCreateButtons();
	}
}


void CNewTaskDialog::OnBnClickedNewTaskIncludeSub()
{
	setChanged();
	UpdateCreateButtons();
}
