#pragma once

#include <atomic>

#include "DBDef.h"
#include "StringT.h"
#include "util.h"
#include "Database.h"
#include "Property.h"
#include "Log.h"
#include "DatePathFormatter.h"
#include "MD5Hash.h"
#include "RepSource.h"

enum class RunnerState
{
	RUNNING,
	STOP
};

using RunnerEventCallback = std::function < void(RunnerState, LPCTSTR) >;

class RepRunner
{
public:
	RepRunner(int taskID, RunnerEventCallback callback = nullptr, bool verbose = false, bool testRun = false);
	~RepRunner();

	void AsyncRun();
	void Run();
	void Abort() { m_abort = true; }
	bool IsRunning() { return m_isRunning;  }

protected:
	int m_taskID;
	bool m_verbose;
	bool m_testRun;
	bool m_isRunning;
	Database::Database m_db;
	Log::Log m_log;
	RunnerEventCallback m_callback;
	DatePathFormatter m_pathFormatter;

	// async execution
	std::atomic_bool m_abort;
	std::thread m_thread;
	static void Thread(RepRunner* _This);

	// member functions
	RepSource GetSource(const PathT& path, int flags, bool matchExtension, const RegexT& filterRegex, int& fileCount);
	void AddPath(const PathT& p, RepSource& repSource, int flags, bool matchExtension, const RegexT& filterRegex, int& fileCount);
	void UpdateTaskInDB(int taskID, Database::PropertyList& propList);
	void AddHistory(Database::PropertyList& propList);
	void ReplicateNow(const std::vector<RepSource>& repSources, const PathT& destination, int flags, int& added, int& updated, int& skipped);
	bool GetNewFileName(PathT& destFile, const Util::MD5Hash& md5);
	void WriteLog(Log::LogLevel level, LPCTSTR format, ...);
};

