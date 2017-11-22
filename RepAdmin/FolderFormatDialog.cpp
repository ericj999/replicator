// FolderFormatDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FolderFormatDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "DatePathFormatter.h"

enum
{
	DFF_DAY_2D = 0,
	DFF_DAY_ALT,
	DFF_WEEK_SHORT,
	DFF_WEEK_LONG,
	DFF_MONTH_2D,
	DFF_MONTH_SHORT,
	DFF_MONTH_LONG,
	DFF_YEAR_2D,
	DFF_YEAR_4D,
	DFF_YEAR_ALT,
	MAX_DATE_FORMATS
};

struct FormatResource
{
	int resId;
	LPCTSTR macro;
} MacroFormats[MAX_DATE_FORMATS] =
{
	{ IDS_FF_DAY_2D, STR_FF_DAY_2D },
	{ IDS_FF_DAY_ALT, STR_FF_DAY_ALT },
	{ IDS_FF_WEEK_SHORT, STR_FF_WEEK_SHORT },
	{ IDS_FF_WEEK_LONG, STR_FF_WEEK_LONG },
	{ IDS_FF_MONTH_2D, STR_FF_MONTH_2D },
	{ IDS_FF_MONTH_SHORT, STR_FF_MONTH_SHORT },
	{ IDS_FF_MONTH_LONG, STR_FF_MONTH_LONG },
	{ IDS_FF_YEAR_2D, STR_FF_YEAR_2D },
	{ IDS_FF_YEAR_4D, STR_FF_YEAR_4D },
	{ IDS_FF_YEAR_ALT, STR_FF_YEAR_ALT }
};

LPCTSTR STR_DEFAULT_FORMAT = _T("$(YYYY)\\$(MMMM)\\$(DD) $(W)");


// FolderFormatDialog dialog

IMPLEMENT_DYNAMIC(FolderFormatDialog, CDialogEx)

FolderFormatDialog::FolderFormatDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FOLDER_FORMATTER, pParent)
{

}

FolderFormatDialog::~FolderFormatDialog()
{
}

void FolderFormatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AF_MACROS, m_macros);
	DDX_Control(pDX, IDC_AF_FORMAT, m_editFormat);
}


BEGIN_MESSAGE_MAP(FolderFormatDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &FolderFormatDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_AF_INSERT, &FolderFormatDialog::OnBnClickedAfInsert)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_AF_MACROS, &FolderFormatDialog::OnLvnItemchangedAfMacros)
	ON_EN_CHANGE(IDC_AF_FORMAT, &FolderFormatDialog::OnEnChangeAfFormat)
END_MESSAGE_MAP()


// FolderFormatDialog message handlers


void FolderFormatDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void FolderFormatDialog::OnBnClickedAfInsert()
{
	POSITION pos = m_macros.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_macros.GetNextSelectedItem(pos);
		CString str = m_macros.GetItemText(index, 0);
		m_editFormat.ReplaceSel(str);
	}
}


BOOL FolderFormatDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_format.IsEmpty())
		m_format = STR_DEFAULT_FORMAT;

	SetDlgItemText(IDC_AF_FORMAT, m_format);

	// list control
	DWORD dwStyles = m_macros.GetExtendedStyle();
	dwStyles |= LVS_EX_FULLROWSELECT;
	m_macros.SetExtendedStyle(dwStyles);
	CRect rect;
	m_macros.GetClientRect(&rect);

	m_macros.InsertColumn(0, _T(""), LVCFMT_LEFT, rect.Width() / 5);
	m_macros.InsertColumn(1, _T(""), LVCFMT_LEFT, rect.Width() - (rect.Width() / 5));

	for (int item = 0; item < MAX_DATE_FORMATS; ++item)
	{
		CString str;

		str.Format(_T("$(%s)"), MacroFormats[item].macro);
		int index = m_macros.InsertItem(item, str);
		str.LoadString(MacroFormats[item].resId);
		m_macros.SetItemText(index, 1, str);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void FolderFormatDialog::OnLvnItemchangedAfMacros(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	GetDlgItem(IDC_AF_INSERT)->EnableWindow((pNMLV->uNewState & LVIS_SELECTED) ? TRUE : FALSE);

	*pResult = 0;
}


void FolderFormatDialog::OnEnChangeAfFormat()
{
	CString str;
	GetDlgItem(IDC_AF_FORMAT)->GetWindowText(str);

	if (!str.IsEmpty())
	{
		DatePathFormatter dpf;
		dpf.SetFormat(StringT((LPCTSTR)str));

		time_t t = time(nullptr);
		struct tm tms;

		localtime_s(&tms, &t);

		GetDlgItem(IDC_AF_FORMAT_EXAMPLE)->SetWindowText(dpf.GetPath(&tms).c_str());
	}
}
