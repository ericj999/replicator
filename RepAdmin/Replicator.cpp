
// Replicator.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <filesystem>
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Replicator.h"
#include "MainFrm.h"

#include "TaskListView.h"
#include <iostream>
#include "util.h"
#include "CRepCommandLineInfo.h"
#include "Table.h"
#include "LocaleResources.h"

#include "StoreFuncs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define REGSTR_KEY_CONFIG_ROOT		_T("SOFTWARE\\Replicator\\Config")
#define REGSTR_VALUE_HISTORY_DAYS	_T("HistoryDays")
#define REGSTR_VALUE_SETTING_FLAGS	_T("Flags")

#define STR_LOG_FILENAME			_T("Replicator.log")

// CReplicatorApp

BEGIN_MESSAGE_MAP(CReplicatorApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CReplicatorApp::OnAppAbout)
	// Standard file based document commands
END_MESSAGE_MAP()


// CReplicatorApp construction

CReplicatorApp::CReplicatorApp() :
	m_verbose{ false }, m_testRun{ false }, m_historyDays{ DEFAULT_HISTORY_DAYS }
{
	m_bHiColorIcons = TRUE;
	m_bSaveState = FALSE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Replicator.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CReplicatorApp object

CReplicatorApp theApp;


// CReplicatorApp initialization

BOOL CReplicatorApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialize global logger
	PathT logPath = Util::GetConfigPath();
	logPath /= STR_LOG_FILENAME;

	Log::logger.setPath(logPath);
	Log::logger.setLevel(Log::LogLevel::Verbose);
	Log::logger.info(_T("Program started."));

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();
	InitLocaleResource();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Replicator"));
	ReadConfig();

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	try
	{
		InitConfigPath();

		PathT dbPath{ Util::GetDatabasePath() };

		if (std::experimental::filesystem::exists(dbPath))
			m_database.Connect(dbPath.wstring());
		else
			CreateDB(dbPath.wstring());

		MaintainDB();
	}
	catch (std::exception& e)
	{
		Log::logger.error(StringT(_T("Failed to connect to the database!")) + String::StringToStringT(e.what()));
		return FALSE;
	}

	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
	{
		Log::logger.error(_T("Failed to connect to main frame window!"));
		return FALSE;
	}
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW, NULL, NULL);

	// Parse command line for standard shell commands, DDE, file open
	CRepCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	m_verbose = cmdInfo.m_verbose;
	m_testRun = cmdInfo.m_testRun;

	CMFCMenuBar::SetRecentlyUsedMenus(FALSE);
	CMFCMenuBar::SetShowAllCommands(TRUE);

	// Check license
/*
	if (!StoreIsRegistered(m_pMainWnd->GetSafeHwnd()))
	{
		AfxMessageBox(IDS_ERROR_LICENSE, MB_ICONSTOP | MB_OK);
		return FALSE;
	}
*/
	// Initialize OLE libraries
	// using STA due to the portable device access
	CoInitialize(NULL);

	// The one and only window has been initialized, so show and update it
	((CMainFrame*)m_pMainWnd)->LoadSettings();
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	Log::logger.info(_T("Initializing instance completed"));
	return TRUE;
}

int CReplicatorApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	CoUninitialize();
	Log::logger.info(_T("Exit instance."));
	return CWinAppEx::ExitInstance();
}

// CReplicatorApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CReplicatorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CReplicatorApp customization load/save methods

void CReplicatorApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_TASK_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_TASK_POPUP_MENU);
}

void CReplicatorApp::LoadCustomState()
{
}

void CReplicatorApp::SaveCustomState()
{
}

// CReplicatorApp message handlers
void CReplicatorApp::InitConfigPath(void)
{
	m_configPath = Util::GetConfigPath();
	TRACE(_T("Config Path = %s\n"), m_configPath.wstring().c_str());
}

void CReplicatorApp::CreateDB(const StringT& db)
{
	m_database.Connect(db);

//	m_database.Exec("PRAGMA auto_vacuum=1;");

	m_database.Exec("CREATE TABLE Tasks(" \
		"TaskID INTEGER PRIMARY KEY AUTOINCREMENT," \
		"Name TEXT NOT NULL," \
		"CreatedTime DATETIME DEFAULT CURRENT_TIMESTAMP," \
		"Source TEXT NOT NULL," \
		"SourceParsing TEXT," \
		"Destination TEXT NOT NULL," \
		"DestParsing TEXT," \
		"Flags INT DEFAULT 0," \
		"Filters TEXT," \
		"LastRun DATETIME," \
		"Status TEXT," \
		"DestFolderFormat TEXT," \
		"LastSuccessfulRun DATETIME" \
		");");

	m_database.Exec("CREATE TABLE History(" \
		"rid INTEGER PRIMARY KEY AUTOINCREMENT," \
		"TaskID INT NOT NULL," \
		"StartTime DATETIME NOT NULL," \
		"EndTime DATETIME," \
		"Result TEXT" \
		");");
}

void CReplicatorApp::MaintainDB()
{
	if (m_historyDays > HISTORY_DAYS_MIN)
	{
		Database::Table history{ GetDB() , HISTORY_TABLE };
		StringStreamT condition;
		
		condition << HISTORY_COL_START_TIME << " < date('now', '-" << m_historyDays << " day')";

		Log::logger.info(StringT(_T("Deleting history older than ")) + ToStringT(m_historyDays) + StringT(_T(" day(s)")));
		history.Delete(condition.str());
		Log::logger.info(_T("Done deleting old history"));
	}
}


void CReplicatorApp::ReadConfig()
{
	CRegKey regKey;

	if (regKey.Create(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_ROOT) == ERROR_SUCCESS)
	{
		DWORD days = DEFAULT_HISTORY_DAYS;
		if ((regKey.QueryDWORDValue(REGSTR_VALUE_HISTORY_DAYS, days) == ERROR_SUCCESS) &&
			(days >= HISTORY_DAYS_MIN) && (days <= HISTORY_DAYS_MAX))
		{
			m_historyDays = static_cast<int>(days);
		}
	}
}

void CReplicatorApp::setHistoryDays(int days)
{
	if (m_historyDays != days)
	{
		CRegKey regKey;

		if (regKey.Create(HKEY_CURRENT_USER, REGSTR_KEY_CONFIG_ROOT) == ERROR_SUCCESS)
		{
			m_historyDays = days;
			if (regKey.SetDWORDValue(REGSTR_VALUE_HISTORY_DAYS, m_historyDays) != ERROR_SUCCESS)
			{
				Log::logger.error(StringT(_T("Failed to update history registry setting. Code:")) + ToStringT(GetLastError()));
			}
		}
	}
}
