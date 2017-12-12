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
#include "ShellFolder.h"

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

	// replication status properties
	int m_fileCount, m_updated, m_skipped, m_added;
	int m_flags;
	bool m_matchExtension;
	RegexT m_filterRegex;

	PathT m_srcPath;
	PathT m_destination;
	StringT m_parsingSrc;


	// async execution
	std::atomic_bool m_abort;
	std::thread m_thread;
	static void Thread(RepRunner* _This);

	// member functions
	RepSource GetSource(const PathT& path);
	void AddPath(const PathT& p, RepSource& repSource);
	void UpdateTaskInDB(int taskID, Database::PropertyList& propList);
	void AddHistory(Database::PropertyList& propList);
	void ReplicateNow(const std::vector<RepSource>& repSources, const PathT& destination);
	bool GetNewFileName(PathT& destFile, const Util::MD5Hash& md5);
	void WriteLog(Log::LogLevel level, LPCTSTR format, ...);
	void ReplicateWithShell(const StringT& parsingSrc, const PathT& srcPath, const PathT& destination);
	void ReplicateWithShell(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination);

	void ReplicateWithShell2(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination);
	void ProcessFiles(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList);
	void ProcessFolders(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList);
};

