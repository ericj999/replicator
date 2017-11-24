
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

#define REFRESH_GENERAL 0x01
#define REFRESH_HISTORY 0x02
#define REFRESH_ALL (REFRESH_GENERAL | REFRESH_HISTORY)

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
//	void WriteLog(int taskID, LogLevel level, const StringT& msg);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

protected:
	bool m_verbose;
	bool m_testRun;

	PathT m_configPath;
	Database::Database m_database;

	void InitConfigPath(void);
	void CreateDB(const StringT& db);

};

extern CReplicatorApp theApp;
