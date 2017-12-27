
// Replicator.h : main header file for the Replicator application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "Database.h"
#include "DBDef.h"
#include "StringT.h"
#include "Log.h"

#define REFRESH_GENERAL 0x01
#define REFRESH_HISTORY 0x02
#define REFRESH_ALL (REFRESH_GENERAL | REFRESH_HISTORY)

#define DEFAULT_HISTORY_DAYS	30
#define HISTORY_DAYS_MIN		0
#define HISTORY_DAYS_MAX		9999

// CReplicatorApp:
// See Replicator.cpp for the implementation of this class
//

class CReplicatorApp : public CWinAppEx
{
public:
	CReplicatorApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	Database::Database& GetDB() { return m_database; }

	bool getVerboseMode() { return m_verbose; }
	bool getTestRunMode() { return m_testRun;  }
	void setTestRunMode(bool enable) { m_testRun = enable; }

	int getHistoryDays() { return m_historyDays;  }
	void setHistoryDays(int days);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

protected:
	bool m_verbose;
	bool m_testRun;
	int m_historyDays;

	PathT m_configPath;
	Database::Database m_database;

	void InitConfigPath(void);
	void CreateDB(const StringT& db);
	void MaintainDB();
	void ReadConfig();
};

extern CReplicatorApp theApp;
