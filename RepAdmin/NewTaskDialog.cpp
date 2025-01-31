// NewTaskDialog.cpp : implementation file
//

#include "stdafx.h"
#include <Shobjidl.h>

#include "Replicator.h"
#include "NewTaskDialog.h"
#include "afxdialogex.h"
#include "StringT.h"
#include "Table.h"
#include "DBDef.h"

#include "AdvancedOptionsDlg.h"
#include "ComInterface.h"

struct tagEditColumnDef
{
	LPCTSTR colDBName;
	Database::PropertyType colDBType;
} EditColumnDef[] =
{
	{ TASKS_COL_TASKID, Database::PT_INT64 },
{ TASKS_COL_NAME, Database::PT_TEXT },
{ TASKS_COL_SOURCE, Database::PT_TEXT },
{ TASKS_COL_SOURCE_PARSING, Database::PT_TEXT },
{ TASKS_COL_DESTINATION, Database::PT_TEXT },
{ TASKS_COL_DEST_PARSING, Database::PT_TEXT },
{ TASKS_COL_FLASGS, Database::PT_INT },
{ TASKS_COL_FILTERS, Database::PT_TEXT },
{ TASKS_COL_DESTFOLDERFMT, Database::PT_TEXT }
};

// CNewTaskDialog dialog

IMPLEMENT_DYNAMIC(CNewTaskDialog, CDialogEx)

CNewTaskDialog::CNewTaskDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewTaskDialog::IDD, pParent), m_update{ false }, m_AdvnacedOptionsFlags{ TASKS_FLAGS_UPDATE_NEWER },
	m_changed{ false }, m_ready{ false }
{

}

CNewTaskDialog::CNewTaskDialog(int taskId, CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewTaskDialog::IDD, pParent), m_update{ true }, m_AdvnacedOptionsFlags{ TASKS_FLAGS_UPDATE_NEWER }, 
	m_changed{ false }, m_ready{ false }
{
	Database::Table tb{ theApp.GetDB(), TASKS_TABLE };
	StringT condition = _T("TaskID=") + ToStringT(taskId);

	for (int i = 0; i < (sizeof(EditColumnDef) / sizeof(EditColumnDef[0])); ++i)
		m_props.push_back(Database::Property(EditColumnDef[i].colDBName, EditColumnDef[i].colDBType));

	Database::RecordsetPtr rs = tb.Select(m_props, condition);
	if (!rs->Step())
	{
		throw std::runtime_error{ EXCEPSTR_EDIT_TASK_FAILURE };
	}
}


CNewTaskDialog::~CNewTaskDialog()
{
}

void CNewTaskDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEW_TASK_SOURCE, m_editSource);
	DDX_Control(pDX, IDC_NEW_TASK_NAME, m_editName);
	DDX_Control(pDX, IDC_NEW_TASK_DESTINATION, m_editDest);
}


BEGIN_MESSAGE_MAP(CNewTaskDialog, CDialogEx)
	ON_BN_CLICKED(IDC_NEW_TASK_BROWSE_DEST, &CNewTaskDialog::OnBnClickedNewtaskBrowseDest)
	ON_BN_CLICKED(IDOK, &CNewTaskDialog::OnBnClickedOk)
	ON_EN_CHANGE(IDC_NEW_TASK_NAME, &CNewTaskDialog::OnEnChange)
	ON_EN_CHANGE(IDC_NEW_TASK_SOURCE, &CNewTaskDialog::OnEnChange)
	ON_EN_CHANGE(IDC_NEW_TASK_DESTINATION, &CNewTaskDialog::OnEnChangeNewTaskDestination)
	ON_BN_CLICKED(IDC_NEW_TASK_CREATE, &CNewTaskDialog::OnBnClickedNewTaskCreate)
	ON_BN_CLICKED(IDC_NEW_TASK_ADV_OPTIONS, &CNewTaskDialog::OnBnClickedNewTaskAdvOptions)
	ON_BN_CLICKED(IDC_NEW_TASK_INCLUDE_SUB, &CNewTaskDialog::OnBnClickedNewTaskIncludeSub)
	ON_BN_CLICKED(IDC_NEW_TASK_BROWSE_SOURCE, &CNewTaskDialog::OnBnClickedNewTaskBrowseSource)
END_MESSAGE_MAP()


// CNewTaskDialog message handlers


BOOL CNewTaskDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CRect rc;

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
		if (!srcProp.IsNULL()) m_editSource.SetWindowText(srcProp.m_str.c_str());

		const Database::Property& srcParsingProp = m_props.Find(TASKS_COL_SOURCE_PARSING);
		if (!srcParsingProp.IsNULL()) m_strSourceParsing = srcParsingProp.m_str.c_str();

		const Database::Property& dest = m_props.Find(TASKS_COL_DESTINATION);
		if (!dest.IsNULL()) m_editDest.SetWindowText(dest.m_str.c_str());

		const Database::Property& destParsingProp = m_props.Find(TASKS_COL_DEST_PARSING);
		if (!destParsingProp.IsNULL()) m_strDestParsing = destParsingProp.m_str.c_str();

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
			if (flags.m_i & TASKS_FLAGS_INCLUDE_SUBDIR)
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

void CNewTaskDialog::OnBnClickedNewtaskBrowseDest()
{
	StringT path, parsingPath;

	if (BrowseFolder(path, parsingPath))
	{
		SetDlgItemText(IDC_NEW_TASK_DESTINATION, path.c_str());
		m_strDestParsing = parsingPath.c_str();

		setChanged();
		UpdateCreateButtons();
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

void CNewTaskDialog::OnEnChangeNewTaskDestination()
{
	setChanged();
	UpdateCreateButtons();
}

void CNewTaskDialog::UpdateCreateButtons()
{
	BOOL bEnable = (m_changed && (m_editName.GetWindowTextLength() > 0) && (m_editDest.GetWindowTextLength() > 0) && (m_editSource.GetWindowTextLength() > 0)) ? TRUE : FALSE;
	GetDlgItem(IDC_NEW_TASK_CREATE)->EnableWindow(bEnable);
}

void CNewTaskDialog::OnBnClickedNewTaskCreate()
{
	try
	{
		if ((m_strSourceParsing.GetLength() > 0) && (m_strSourceParsing.GetAt(0) == L':')
			&& (m_strDestParsing.GetLength() > 0) && (m_strDestParsing.GetAt(0) == L':'))
		{
			AfxMessageBox(IDS_ERROR_PORTABLE_TO_PORTABLE, MB_ICONSTOP | MB_OK);
			return;
		}
		CString strName, strSource, strDest;
		m_editName.GetWindowText(strName);
		m_editSource.GetWindowText(strSource);
		m_editDest.GetWindowText(strDest);

		int flags = 0;
		CButton* pBtn = static_cast<CButton*>(GetDlgItem(IDC_NEW_TASK_INCLUDE_SUB));
		if (pBtn && (pBtn->GetCheck() == BST_CHECKED))
			flags |= TASKS_FLAGS_INCLUDE_SUBDIR;

		flags |= m_AdvnacedOptionsFlags;

		Database::PropertyList propList;
		Database::Table table{ theApp.GetDB(), TASKS_TABLE };
		if (m_update)
		{
			if (m_props.Find(TASKS_COL_NAME).m_str.c_str() != strName)
				propList.push_back(Database::Property(TASKS_COL_NAME, strName));

			if (m_props.Find(TASKS_COL_SOURCE).m_str.c_str() != strSource)
				propList.push_back(Database::Property(TASKS_COL_SOURCE, strSource));

			if (m_props.Find(TASKS_COL_SOURCE_PARSING).m_str.c_str() != m_strSourceParsing)
				propList.push_back(Database::Property(TASKS_COL_SOURCE_PARSING, m_strSourceParsing));

			if (m_props.Find(TASKS_COL_DESTINATION).m_str.c_str() != strDest)
				propList.push_back(Database::Property(TASKS_COL_DESTINATION, strDest));

			if (m_props.Find(TASKS_COL_DEST_PARSING).m_str.c_str() != m_strDestParsing)
				propList.push_back(Database::Property(TASKS_COL_DEST_PARSING, m_strDestParsing));

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
			propList.push_back(Database::Property(TASKS_COL_SOURCE, strSource));
			propList.push_back(Database::Property(TASKS_COL_SOURCE_PARSING, m_strSourceParsing));
			propList.push_back(Database::Property(TASKS_COL_DESTINATION, strDest));
			propList.push_back(Database::Property(TASKS_COL_DEST_PARSING, m_strDestParsing));
			propList.push_back(Database::Property(TASKS_COL_DESTFOLDERFMT, m_strDestinationFolderFormat));
			propList.push_back(Database::Property(TASKS_COL_FLASGS, flags));
			propList.push_back(Database::Property(TASKS_COL_FILTERS, m_strFilters));

			table.Insert(propList);
		}
		m_strNewTaskName = strName;
	}
	catch (Database::Exception& e)
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
	if ((m_strDestParsing.GetLength() > 0) && (m_strDestParsing.GetAt(0) == L':'))
		advDlg.setNoDestinationOptions();

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

void CNewTaskDialog::OnBnClickedNewTaskBrowseSource()
{
	StringT path, parsingPath;

	if (BrowseFolder(path, parsingPath))
	{
		m_editSource.SetWindowText(path.c_str());
		m_strSourceParsing = parsingPath.c_str();

		setChanged();
		UpdateCreateButtons();
	}
}

bool CNewTaskDialog::BrowseFolder(StringT& pathName, StringT& parsingPathName)
{
	bool ret = false;
	ComInterface<IFileOpenDialog> fileOpenDlg{ CLSID_FileOpenDialog , IID_IFileOpenDialog };
	FILEOPENDIALOGOPTIONS fos = 0;
	fileOpenDlg->GetOptions(&fos);
	fos |= FOS_PICKFOLDERS | FOS_PATHMUSTEXIST;
	fileOpenDlg->SetOptions(fos);

	HRESULT hr = fileOpenDlg->Show(GetSafeHwnd());
	if (SUCCEEDED(hr))
	{
		IShellItem* pItem = nullptr;
		hr = fileOpenDlg->GetResult(&pItem);
		if (SUCCEEDED(hr))
		{
			PWSTR pszFilePath = nullptr;
			hr = pItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEEDITING, &pszFilePath);
			if (SUCCEEDED(hr) && pszFilePath)
			{
				StringT source = pszFilePath;
				CoTaskMemFree(pszFilePath);
				pszFilePath = nullptr;

				hr = pItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
				if (SUCCEEDED(hr) && pszFilePath)
				{
					StringT sourceParsing = pszFilePath;
					CoTaskMemFree(pszFilePath);
					pszFilePath = nullptr;

					SFGAOF attr = 0;
					if (SUCCEEDED((hr = pItem->GetAttributes(SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, &attr))))
					{
						pathName = source;
						parsingPathName = sourceParsing;
						ret = true;
					}
				}
			}
			pItem->Release();
		}
	}
/*
	if (FAILED(hr))
	{
		StringT msg{ _T("Failed to select a folder. Code:") };
		msg += ToStringT(hr);

		theApp.getLog().error(msg);
		AfxMessageBox(IDS_ERROR_SELECT_SOURCE, MB_OK | MB_ICONEXCLAMATION);
	}
*/
	return ret;
}
