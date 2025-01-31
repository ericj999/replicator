
// MFCApplication1Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "MessageDialog.h"

#include <Shobjidl.h>
#include <sstream>
#include <filesystem>
#include "WPDPortableDeviceManager.h"
#include "WPDPortableDevice.h"
#include "WPDPortableDeviceContent.h"
#include "WPDPortableDeviceProperties.h"
#include "ShellFolder.h"

#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1Dlg dialog



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_IFILEDIALOG, &CMFCApplication1Dlg::OnBnClickedIfiledialog)
	ON_BN_CLICKED(IDC_FILEDIALOG, &CMFCApplication1Dlg::OnBnClickedFiledialog)
	ON_BN_CLICKED(IDC_FOLDERPICKERDIALOG, &CMFCApplication1Dlg::OnBnClickedFolderpickerdialog)
	ON_BN_CLICKED(IDC_SHCreateItemFromParsingName, &CMFCApplication1Dlg::OnBnClickedShcreateitemfromparsingname)
	ON_BN_CLICKED(IDC_GET_PORTABLE_DEVICES, &CMFCApplication1Dlg::OnBnClickedGetPortableDevices)
	ON_BN_CLICKED(IDOK, &CMFCApplication1Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ENUM_FOLDER_PATH, &CMFCApplication1Dlg::OnBnClickedEnumFolderPath)
	ON_BN_CLICKED(IDC_GET_DEVICE, &CMFCApplication1Dlg::OnBnClickedGetDevice)
	ON_BN_CLICKED(IDC_OPEN_FOLDER, &CMFCApplication1Dlg::OnBnClickedOpenFolder)
	ON_BN_CLICKED(IDC_RELATIVE_PATH, &CMFCApplication1Dlg::OnBnClickedRelativePath)
	ON_BN_CLICKED(IDC_FILENAME, &CMFCApplication1Dlg::OnBnClickedFilename)
	ON_BN_CLICKED(IDC_PARSEPARSINGPATH, &CMFCApplication1Dlg::OnBnClickedParseparsingpath)
	ON_BN_CLICKED(IDC_CREATE_FOLDER, &CMFCApplication1Dlg::OnBnClickedCreateFolder)
END_MESSAGE_MAP()


// CMFCApplication1Dlg message handlers

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

struct ShellItemInfo
{
	LPCTSTR name;
	SIGDN sigdn;
} si[] =
{
	{ L"SIGDN_NORMALDISPLAY", SIGDN_NORMALDISPLAY },
	{ L"SIGDN_PARENTRELATIVEPARSING", SIGDN_PARENTRELATIVEPARSING },
	{ L"SIGDN_DESKTOPABSOLUTEPARSING", SIGDN_DESKTOPABSOLUTEPARSING},
	{ L"SIGDN_PARENTRELATIVEEDITING", SIGDN_PARENTRELATIVEEDITING},
	{ L"SIGDN_DESKTOPABSOLUTEEDITING", SIGDN_DESKTOPABSOLUTEEDITING},
	{ L"SIGDN_FILESYSPATH", SIGDN_FILESYSPATH},
	{ L"SIGDN_URL", SIGDN_URL},
	{ L"SIGDN_PARENTRELATIVEFORADDRESSBAR", SIGDN_PARENTRELATIVEFORADDRESSBAR},
	{ L"SIGDN_PARENTRELATIVE", SIGDN_PARENTRELATIVE},
	{ L"SIGDN_PARENTRELATIVEFORUI", SIGDN_PARENTRELATIVEFORUI}
};

struct ShellItemAttr
{
	LPCTSTR name;
	SFGAOF attr;
} sa[] =
{
	{  L"SFGAO_CANCOPY", SFGAO_CANCOPY },
	{ L"SFGAO_CANMOVE", SFGAO_CANMOVE },
	{ L"SFGAO_CANLINK", SFGAO_CANLINK },
	{ L"SFGAO_STORAGE", SFGAO_STORAGE },
	{ L"SFGAO_CANRENAME", SFGAO_CANRENAME },
	{ L"SFGAO_CANDELETE", SFGAO_CANDELETE },
	{ L"SFGAO_HASPROPSHEET", SFGAO_HASPROPSHEET },
	{ L"SFGAO_DROPTARGET", SFGAO_DROPTARGET },
	{ L"SFGAO_PLACEHOLDER", SFGAO_PLACEHOLDER },
	{ L"SFGAO_SYSTEM", SFGAO_SYSTEM },
	{ L"SFGAO_ENCRYPTED", SFGAO_ENCRYPTED },
	{ L"SFGAO_ISSLOW", SFGAO_ISSLOW },
	{ L"SFGAO_GHOSTED", SFGAO_GHOSTED },
	{ L"SFGAO_LINK", SFGAO_LINK },
	{ L"SFGAO_SHARE", SFGAO_SHARE },
	{ L"SFGAO_READONLY", SFGAO_READONLY },
	{ L"SFGAO_HIDDEN", SFGAO_HIDDEN },
	{ L"SFGAO_FILESYSANCESTOR", SFGAO_FILESYSANCESTOR },
	{ L"SFGAO_FOLDER", SFGAO_FOLDER },
	{ L"SFGAO_FILESYSTEM", SFGAO_FILESYSTEM },
	{ L"SFGAO_HASSUBFOLDER", SFGAO_HASSUBFOLDER },
	{ L"SFGAO_VALIDATE", SFGAO_VALIDATE },
	{ L"SFGAO_REMOVABLE", SFGAO_REMOVABLE },
	{ L"SFGAO_COMPRESSED", SFGAO_COMPRESSED },
	{ L"SFGAO_BROWSABLE", SFGAO_BROWSABLE },
	{ L"SFGAO_NONENUMERATED", SFGAO_NONENUMERATED },
	{ L"SFGAO_NEWCONTENT", SFGAO_NEWCONTENT },
	{ L"SFGAO_CANMONIKER", SFGAO_CANMONIKER },
	{ L"SFGAO_HASSTORAGE", SFGAO_HASSTORAGE },
	{ L"SFGAO_STREAM", SFGAO_STREAM },
	{ L"SFGAO_STORAGEANCESTOR", SFGAO_STORAGEANCESTOR }
};

//#define SFGAO_CAPABILITYMASK    0x00000177L
//#define SFGAO_DISPLAYATTRMASK   0x000FC000L
//#define SFGAO_CONTENTSMASK      0x80000000L
//#define SFGAO_STORAGECAPMASK    0x70C50008L 


void CMFCApplication1Dlg::OnBnClickedIfiledialog()
{
	ComInterface<IFileOpenDialog> fileOpenDlg{ CLSID_FileOpenDialog , IID_IFileOpenDialog };

	FILEOPENDIALOGOPTIONS fos = 0;
	fileOpenDlg->GetOptions(&fos);
	fos |= FOS_PICKFOLDERS;
	fileOpenDlg->SetOptions(fos);

	// Show the Open dialog box.
	HRESULT hr = fileOpenDlg->Show(NULL);

	// Get the file name from the dialog box.
	if (SUCCEEDED(hr))
	{
		IShellItem *pItem;
		hr = fileOpenDlg->GetResult(&pItem);
		if (SUCCEEDED(hr))
		{

			LPITEMIDLIST pidl = nullptr;
			hr = SHGetIDListFromObject(pItem, &pidl);
			if (SUCCEEDED(hr))
			{
				CoTaskMemFree(pidl);
			}

			CString msg;

			SFGAOF attr = 0;

			if (SUCCEEDED(pItem->GetAttributes(SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, &attr)))
			{
				msg += L"Attributes: ";
				for (int i = 0, ii = 0; i < sizeof(sa) / sizeof(ShellItemAttr); ++i)
				{
					if (attr & sa[i].attr)
					{
						if (ii > 0)
							msg += L", ";

						++ii;
						msg += sa[i].name;
					}
				}
				msg += L"\r\n";
			}

			for (int i = 0; i < sizeof(si) / sizeof(ShellItemInfo); ++i)
			{
				CString pathInfo;
				PWSTR pszFilePath = NULL;
				hr = pItem->GetDisplayName(si[i].sigdn, &pszFilePath);
				if (SUCCEEDED(hr))
				{
					pathInfo.Format(L"%s=%s\r\n", si[i].name, pszFilePath);
				}
				else
				{
					pathInfo.Format(L"%s=%0x\r\n", si[i].name, hr);
				}
				msg += pathInfo;
				if(pszFilePath) CoTaskMemFree(pszFilePath);
			}
			pItem->Release();

			ShowMessage(msg, this);
		}
	}
}
	


void CMFCApplication1Dlg::OnBnClickedFiledialog()
{
	CFileDialog fileDialog(TRUE);

	if (fileDialog.DoModal() == IDOK)
	{
		AfxMessageBox(fileDialog.GetPathName());
	}

}


void CMFCApplication1Dlg::OnBnClickedFolderpickerdialog()
{
	CFolderPickerDialog fpg(NULL, 0, this);
	if (fpg.DoModal() == IDOK)
	{
	}
}


void CMFCApplication1Dlg::OnBnClickedShcreateitemfromparsingname()
{
	CString str;
	HRESULT hr;

	GetDlgItemText(IDC_PARSINGNAME, str);

	IShellItem* pItem = NULL;
	if (SUCCEEDED((hr = SHCreateItemFromParsingName(str, NULL, IID_IShellItem, (void**)&pItem))))
	{
		PWSTR pszFilePath = NULL;
		if (SUCCEEDED((hr = pItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEEDITING, &pszFilePath))))
		{
			AfxMessageBox(pszFilePath);
		}
		else
		{
			AfxMessageBox(L"Failed!");
		}
		if (pszFilePath) CoTaskMemFree(pszFilePath);
		pItem->Release();
	}
	else
	{
		CString err;
		err.Format(L"Error code: 0x%0x", hr);
		AfxMessageBox(err);
	}
}


void CMFCApplication1Dlg::OnBnClickedGetPortableDevices()
{
	WPD::PortableDeviceManager manager;

	std::wstringstream msg;

	std::vector<std::wstring> deviceIdList = manager.GetDevices();
	for (auto id : deviceIdList)
	{
		msg << L"Manafacturer:" << manager.getDeviceManufacturer(id) << L"\r\n";
		msg << L"Device:" << manager.getDeviceFriendlyName(id) << L"\r\n";
		msg << L"Description:" << manager.getDeviceDescription(id) << L"\r\n";
		msg << L"Device ID:" << id << "\r\n\r\n";
	}
	ShowMessage(msg.str().c_str(), this);
}


void CMFCApplication1Dlg::OnBnClickedOk()
{
}


void CMFCApplication1Dlg::OnBnClickedEnumFolderPath()
{
	CString str;

	GetDlgItemText(IDC_FOLDER_PATH, str);

	ShellWrapper::ShellFolder desktopFolder;

	if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder)))
	{
		std::wstring wstr = str;
//		desktopFolder.EnumShellFolder(wstr);
	}
}


void CMFCApplication1Dlg::OnBnClickedGetDevice()
{
	CString id;
	GetDlgItemText(IDC_WPD_DEVICE_ID, id);

	std::wstring wstr = id;
	WPD::PortableDevice portableDevice{ wstr };

	WPD::PortableDeviceContent content;

	HRESULT hr = portableDevice->Content(&content);


	// get content properties

	WPD::PortableDeviceProperties properties;

	hr = content->Properties(&properties);

	WPD::PortableDeviceKeyCollection keys;

	keys->Add(WPD_OBJECT_PARENT_ID);
	keys->Add(WPD_OBJECT_NAME);
	keys->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
	keys->Add(WPD_OBJECT_FORMAT);
	keys->Add(WPD_OBJECT_CONTENT_TYPE);

	WPD::PortableDeviceValues values;
	hr = properties->GetValues(WPD_DEVICE_OBJECT_ID, keys.Get(), &values);

	DWORD count = 0;
	hr = values->GetCount(&count);

	if (SUCCEEDED(hr))
	{
		for (DWORD i = 0; i < count; ++i)
		{
			PROPERTYKEY key;
			ComPropVariant prop;
			hr = values->GetAt(i, &key, prop.Get());

		}
	}

	return;
}




wchar_t const* GetContentTypeString(const GUID& guid)
{
	if (IsEqualGUID(WPD_CONTENT_TYPE_IMAGE_ALBUM, guid))
		return L"WPD_CONTENT_TYPE_IMAGE_ALBUM";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_MEMO, guid))
		return L"WPD_CONTENT_TYPE_MEMO";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_MIXED_CONTENT_ALBUM, guid))
		return L"WPD_CONTENT_TYPE_MIXED_CONTENT_ALBUM";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_PLAYLIST, guid))
		return L"WPD_CONTENT_TYPE_PLAYLIST";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_ALL, guid))
		return L"WPD_CONTENT_TYPE_ALL";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_APPOINTMENT, guid))
		return L"WPD_CONTENT_TYPE_APPOINTMENT";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_AUDIO_ALBUM, guid))
		return L"WPD_CONTENT_TYPE_AUDIO_ALBUM";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_AUDIO, guid))
		return L"WPD_CONTENT_TYPE_AUDIO";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_CALENDAR, guid))
		return L"WPD_CONTENT_TYPE_CALENDAR";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_CERTIFICATE, guid))
		return L"WPD_CONTENT_TYPE_CERTIFICATE";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_CONTACT, guid))
		return L"WPD_CONTENT_TYPE_CONTACT";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_CONTACT_GROUP, guid))
		return L"WPD_CONTENT_TYPE_CONTACT_GROUP";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_DOCUMENT, guid))
		return L"WPD_CONTENT_TYPE_DOCUMENT";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_EMAIL, guid))
		return L"WPD_CONTENT_TYPE_EMAIL";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_FOLDER, guid))
		return L"WPD_CONTENT_TYPE_FOLDER";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT, guid))
		return L"WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_GENERIC_FILE, guid))
		return L"WPD_CONTENT_TYPE_GENERIC_FILE";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_GENERIC_MESSAGE, guid))
		return L"WPD_CONTENT_TYPE_GENERIC_MESSAGE";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_MEDIA_CAST, guid))
		return L"WPD_CONTENT_TYPE_MEDIA_CAST";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_IMAGE, guid))
		return L"WPD_CONTENT_TYPE_IMAGE";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_NETWORK_ASSOCIATION, guid))
		return L"WPD_CONTENT_TYPE_NETWORK_ASSOCIATION";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_PROGRAM, guid))
		return L"WPD_CONTENT_TYPE_PROGRAM";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_SECTION, guid))
		return L"WPD_CONTENT_TYPE_SECTION";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_TASK, guid))
		return L"WPD_CONTENT_TYPE_TASK";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_TELEVISION, guid))
		return L"WPD_CONTENT_TYPE_TELEVISION";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_UNSPECIFIED, guid))
		return L"WPD_CONTENT_TYPE_UNSPECIFIED";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_VIDEO_ALBUM, guid))
		return L"WPD_CONTENT_TYPE_VIDEO_ALBUM";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_VIDEO, guid))
		return L"WPD_CONTENT_TYPE_VIDEO";
	else if (IsEqualGUID(WPD_CONTENT_TYPE_WIRELESS_PROFILE, guid))
		return L"WPD_CONTENT_TYPE_WIRELESS_PROFILE";
	else
		return L"Unknown";
}


#define TASKS_FLAGS_INCLUDE_SUBDIR			0x00010000
#define ENUM_COUNT 50
#define SHELLITEM_REQUIRED_ATTRIBUTES (SFGAO_CANCOPY | SFGAO_STREAM)


void CMFCApplication1Dlg::OnBnClickedOpenFolder()
{
	CString str;
	int flags = 0;

	GetDlgItemText(IDC_FOLDER_PATH, str);

	ShellWrapper::ShellFolder folder{ std::wstring{str} };

	HRESULT hr = E_FAIL;
	ShellWrapper::EnumIDList enumIDList;

	SHCONTF scf = SHCONTF_NONFOLDERS;
	if (flags & TASKS_FLAGS_INCLUDE_SUBDIR)
		scf |= SHCONTF_FOLDERS;

	if (SUCCEEDED(hr = folder->EnumObjects(NULL, scf, &enumIDList)))
	{
		HRESULT getResult = E_FAIL;
		do
		{
			ULONG get = ENUM_COUNT, got = 0;
			ComMemArray<LPITEMIDLIST, ENUM_COUNT> folderItemIdList;

			getResult = enumIDList->Next(get, folderItemIdList.data(), &got);
			for (ULONG item = 0; item < got; ++item)
			{
				std::wstring srcNormal = folder.GetDisplayNameOf(folderItemIdList[item], SHGDN_NORMAL);
				std::wstring srcForEditing = folder.GetDisplayNameOf(folderItemIdList[item], SHGDN_FOREDITING);
				std::wstring srcForEditingInFolder = folder.GetDisplayNameOf(folderItemIdList[item], SHGDN_FOREDITING | SHGDN_INFOLDER);
				std::wstring srcForParsing = folder.GetDisplayNameOf(folderItemIdList[item], SHGDN_FOREDITING);
				std::wstring srcForParsingInFolder = folder.GetDisplayNameOf(folderItemIdList[item], SHGDN_FOREDITING | SHGDN_INFOLDER);
				std::wstring srcForAddressbar = folder.GetDisplayNameOf(folderItemIdList[item], SHGDN_FORADDRESSBAR);

				ComInterface<IShellItem2> shellItem;
				if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), folderItemIdList[item], IID_IShellItem2, (void**)&shellItem)))
				{
					std::wstring filename;
					
					SFGAOF attrs = 0;
					if (SUCCEEDED(hr = shellItem->GetAttributes(SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, &attrs)))
					{
						std::wstring msg;

						msg += L"Attributes: ";
						for (int i = 0, ii = 0; i < sizeof(sa) / sizeof(ShellItemAttr); ++i)
						{
							if (attrs & sa[i].attr)
							{
								if (ii > 0)
									msg += L", ";

								++ii;
								msg += sa[i].name;
							}
						}
						msg += L"\r\n";
					}

					ComInterface<IShellItem2> item2;
//					if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), folderItemIdList[item], IID_IShellItem2, (void**)&item2)))
					if (SUCCEEDED(hr = shellItem->QueryInterface(IID_IShellItem2, (void**)&item2)))
					{
						FILETIME ft = { 0 };
						if (SUCCEEDED(hr = shellItem->GetFileTime(PKEY_DateModified, &ft)))
						{
							SYSTEMTIME st = { 0 };
							::FileTimeToSystemTime(&ft, &st);
							CString str;
							str.Format(
								L"Last write time: %04d-%02d-%02d %02d:%02d:%02d\n",
								st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
						}
						ULONG attrib = 0;
						if (SUCCEEDED(hr = shellItem->GetUInt32(PKEY_FileAttributes, &attrib)))
						{
							int ff = 0;

						}
					}
				}
			}
		} while ((getResult == S_OK) && SUCCEEDED(hr));
	}
}


void CMFCApplication1Dlg::OnBnClickedRelativePath()
{
	CString str;

	GetDlgItemText(IDC_FOLDER_PATH, str);

	std::experimental::filesystem::path path{ (LPCTSTR) str };
	
	AfxMessageBox(path.relative_path().wstring().c_str(), MB_OK);
}


void CMFCApplication1Dlg::OnBnClickedFilename()
{
	CString str;

	GetDlgItemText(IDC_FOLDER_PATH, str);

	std::experimental::filesystem::path path{ (LPCTSTR)str };

	AfxMessageBox(path.filename().wstring().c_str(), MB_OK);
}


void CMFCApplication1Dlg::OnBnClickedParseparsingpath()
{
	CString str;

	GetDlgItemText(IDC_FOLDER_PATH, str);

	std::wstring wstr = str;

	std::vector<std::wstring> list = Util::ParseParsingPath(wstr);

	std::wstring msg;
	for (std::wstring s : list)
	{
		msg += s + L"\r\n";
	}
	AfxMessageBox(msg.c_str(), MB_OK);
}


void CMFCApplication1Dlg::OnBnClickedCreateFolder()
{
	CString str, subFolder;
	int flags = 0;

	GetDlgItemText(IDC_FOLDER_PATH, str);
	GetDlgItemText(IDC_2ND_PARAM, subFolder);

	ShellWrapper::ShellFolder folder{ std::wstring{ str } };

	ShellWrapper::ShellFolder subItem = folder.CreateSubFolder((LPCTSTR)subFolder);
	if (!subItem)
		AfxMessageBox(L"Failed to create or open the subfolder!");
	else
		AfxMessageBox(L"Folder id valid!", MB_OK);
}
