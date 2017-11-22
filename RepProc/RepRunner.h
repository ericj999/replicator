#pragma once

#include "DBDef.h"
#include "StringT.h"
#include "util.h"
#include "Database.h"
#include "Property.h"
#include "Log.h"

class RepRunner
{
public:
	RepRunner();
	~RepRunner();

	bool ParseCommandLine(int argc, _TCHAR* argv[]);
	void run();

	void WriteLog(Log::LogLevel level, LPCTSTR format, ...);

protected:
	int m_taskID;
	bool m_verbose;
	bool m_testRun;
	Database::Database m_db;
	Log::Log m_log;

	void AddPath(const PathT& p, std::vector<PathT>& paths, int flags, bool matchExtension, const RegexT& filterRegex, int& fileCount);
	void UpdateTaskInDB(int taskID, Database::PropertyList& propList);
	void ReplicateNow(const std::vector<PathT>& paths, const PathT& destination, int flags, int& added, int& updated, int& skipped);
	void GetNewFileName(PathT& destFile);

};

