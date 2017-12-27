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
#include "WPDPortableDeviceContent.h"

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
	StringT m_parsingDestination;

	// async execution
	std::atomic_bool m_abort;
	std::thread m_thread;
	static void Thread(RepRunner* _This);

	// member functions
	RepSource GetSource(const PathT& path);
	bool MatchedExtension(const std::wstring& ext);
	void AddPath(const PathT& p, RepSource& repSource);
	void UpdateTaskInDB(int taskID, Database::PropertyList& propList);
	void AddHistory(Database::PropertyList& propList);
	void ReplicateNow(const std::vector<RepSource>& repSources, const PathT& destination);
	bool GetNewFileName(PathT& destFile, const Util::MD5Hash& md5);
	void WriteLog(Log::LogLevel level, LPCTSTR format, ...);

	// from portable devie to filesystem
	void ReplicateStreamToFile(const StringT& parsingSrc, const PathT& srcPath, const PathT& destination);
	void ReplicateStreamToFile(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination);
	void ProcessStreamFiles(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList);
	void ProcessStreamFolders(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList);

	// from filesystem to portable device using shell api's
	void ReplicateFileToStream(const PathT& srcPath, const PathT& destination, const PathT& parsingDest);
	void ReplicateFileToStream(const PathT& srcPath, ShellWrapper::ShellFolder& folder, const PathT& destination);
	void ProcessFiles(const std::vector<PathT>& fileList, ShellWrapper::ShellFolder& folder, const PathT& destination);
	void ProcessFolders(const std::vector<PathT>& folderList, ShellWrapper::ShellFolder& folder, const PathT& destination);

	// from filesystem to portable device using WPD
	void ReplicateFilesToPortableDevice(const PathT& srcPath, const PathT& destination, const PathT& parsingDest);
	void ReplicateFilesToPortableDevice(WPD::PortableDeviceContent& deviceContent, const PathT& srcPath, const std::wstring& destObjId, const PathT& destination);
	void ProcessFiles(WPD::PortableDeviceContent& deviceContent, const std::vector<PathT>& srcFileList, const std::wstring& currentFolderObjId,
		const PathT& currentDestPath, const WPD::PortableDeviceItemMap& destFileMap);
	void ProcessFolders(WPD::PortableDeviceContent& deviceContent, const std::vector<PathT>& srcFolderList, const std::wstring& currentFolderObjId,
		const PathT& currentDestPath, const std::map<std::wstring, std::wstring>& destFolderMap);


	// replication using shell API's
	//void Replicate(const PathT& parsingSource, const PathT& parsingDest);
};

