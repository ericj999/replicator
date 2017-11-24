#include "stdafx.h"
#include <iostream>
#include <ctime>
#include <locale>
#include <codecvt>
#include "RepRunner.h"
#include "GlobDef.h"
#include "TaskRecord.h"
#include "Table.h"
#include "RepSource.h"

#include "resource.h"

#define STR_LOG_FILENAME	_T("Replicate-")
#define STR_LOG_EXTENSION	_T(".log")

RepRunner::RepRunner(int taskID, RunnerEventCallback callback /* = nullptr*/, bool verbose /*= false*/, bool testRun /*= false*/) :
	m_taskID{ taskID }, m_callback{ callback }, m_verbose{	verbose }, m_testRun{ testRun }, m_abort{ false }, m_isRunning{ false }
{
	PathT logPath = Util::GetConfigPath();
	StringStreamT fn;

	fn << STR_LOG_FILENAME << taskID << STR_LOG_EXTENSION;
	logPath /= fn.str();

	m_log.setPath(logPath);

	PathT dbPath{ Util::GetDatabasePath() };
	m_db.Connect(dbPath);
}

RepRunner::~RepRunner()
{
	Abort();
	if (m_thread.joinable())
		m_thread.join();
}

void RepRunner::Thread(RepRunner* _This)
{
	_This->Run();
}

void RepRunner::AsyncRun()
{
	m_thread = std::thread{ Thread, this };
}

void RepRunner::Run()
{
	m_isRunning = true;
	StringT result;

	try
	{
		m_log.setLevel(Log::LogLevel::Verbose);

		WriteLog(Log::LogLevel::Info, _T("Begin replication..."));
		std::chrono::system_clock::time_point start{ std::chrono::system_clock::now() };
		StringT startTimeStr = Util::GetIsoTimeString(std::chrono::system_clock::to_time_t(start));
		Database::PropertyList propList;

		propList.push_back(Database::Property(TASKS_COL_LASTRUN, startTimeStr));
		propList.push_back(Database::Property(TASKS_COL_LASTRUNSTATUS, _T("Started")));

		UpdateTaskInDB(m_taskID, propList);

		TaskRecord task{ m_db, m_taskID };
		int fileCount = 0;
		std::vector<StringT> sources;
		std::vector<PathT> paths;
		int flags = task.getFlags();

		String::Tokenize(task.getSource(), sources, STR_SRC_PATH_SEPARATOR);

		StringT regexFilters = String::GetFiltersExp(task.getFilters());
		bool matchExtension = (flags & (TASKS_FLAGS_INCLUDE_FILTERS | TASKS_FLAGS_EXCLUDE_FILTERS)) && !regexFilters.empty();
		RegexT filterRegex(regexFilters, std::regex::ECMAScript | std::regex::icase);

		WriteLog(Log::LogLevel::Info, _T("Flags = 0x%X"), flags);
		WriteLog(Log::LogLevel::Info, _T("Filters=\"%s\""), regexFilters.c_str());

		StringT destFolderFormat = task.getDestinationFolderFormat();

		if (flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
		{
			if (destFolderFormat.empty())
			{
				propList.clear();
				propList.push_back(Database::Property(TASKS_COL_LASTRUNSTATUS, _T("Destination folder format is empty.")));

				UpdateTaskInDB(m_taskID, propList);
				throw std::runtime_error("Destination folder format is empty.");
			}
			m_pathFormatter.SetFormat(destFolderFormat);
		}

		std::vector<RepSource> repSources;

		// discovering....
		for (auto&& src : sources)
		{
			PathT srcPath{ src };
			repSources.push_back(GetSource(srcPath, flags, matchExtension, filterRegex, fileCount));
			if (m_abort)
			{
				WriteLog(Log::LogLevel::Info, _T("Replication was aborted."));
				break;
			}
		}

		int updated = 0, skipped = 0, added = 0;

		if (!m_abort)
		{
			WriteLog(Log::LogLevel::Info, _T("Discoverd total %d file(s)."), fileCount);

			// processing...
			PathT destination{ task.getDestination() };
			WriteLog(Log::LogLevel::Info, _T("Destination = \"%s\""), destination.wstring().c_str());

			ReplicateNow(repSources, destination, flags, added, updated, skipped);
		}
		std::chrono::system_clock::time_point endTime{ std::chrono::system_clock::now() };

		propList.clear();

		CString cstr, rstr;
		std::chrono::system_clock::duration dur = endTime - start;
		std::chrono::seconds runTime = std::chrono::duration_cast<std::chrono::seconds>(dur);
		StringT runTimeStr = Util::GetDurationString(runTime);
		StringT endTimeStr = Util::GetIsoTimeString(std::chrono::system_clock::to_time_t(endTime));

		if (!m_abort)
		{
			rstr.LoadString(IDS_COMPLETED);
			if (!m_testRun)
				propList.push_back(Database::Property(TASKS_COL_LASTSUCCESSRUN, endTimeStr));
		}
		else
		{
			rstr.LoadString(IDS_ABORTED);
		}

		cstr.Format(IDS_RESULT_FORMAT, (LPCTSTR) rstr, runTimeStr.c_str(), fileCount, added, updated, skipped);

		result = (LPCTSTR)cstr;
		propList.push_back(Database::Property(TASKS_COL_LASTRUNSTATUS, result));
		UpdateTaskInDB(m_taskID, propList);

		propList.clear();

		propList.push_back(Database::Property(HISTORY_COL_TASKID, m_taskID));
		propList.push_back(Database::Property(HISTORY_COL_START_TIME, startTimeStr));
		propList.push_back(Database::Property(HISTORY_COL_END_TIME, endTimeStr));
		propList.push_back(Database::Property(HISTORY_COL_RESULT, result));
		AddHistory(propList);
	}
	catch (std::exception& e)
	{
		result = _T("Exception occurred. ") + String::StringToStringT(e.what());
		WriteLog(Log::LogLevel::Error, result.c_str() );
	}
	if (m_callback) m_callback(RunnerState::STOP, result.c_str());
	m_isRunning = false;
}

void RepRunner::ReplicateNow(const std::vector<RepSource>& repSources, const PathT& destination, int flags, int& added, int& updated, int& skipped)
{
	for (auto&& source : repSources)
	{
		for (auto&& src : source.getChildren())
		{
			if (m_abort)
			{
				WriteLog(Log::LogLevel::Info, _T("Replication was aborted."));
				return;
			}

			PathT destFile{ destination };
			PathT srcFile{ source.getParent() };

			srcFile /= src;

			auto srcFileTime = std::tr2::sys::last_write_time(srcFile);
			auto srcFileSize = std::tr2::sys::file_size(srcFile);

			if (flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
			{
				std::time_t tt = decltype(srcFileTime)::clock::to_time_t(srcFileTime);
				std::tm ft;
				localtime_s(&ft, &tt);

				destFile /= m_pathFormatter.GetPath(&ft);
				destFile /= srcFile.filename();
			}
			else
			{
				if(flags & TASKS_FLAGS_DEST_START_FROM_ROOT)
					destFile /= srcFile.relative_path();
				else
					destFile /= src.relative_path();
			}

			if (!std::tr2::sys::exists(destFile.parent_path()))
				std::tr2::sys::create_directories(destFile.parent_path());

			bool toUpdate = false, toAddNew = false;

			if (std::tr2::sys::exists(destFile))
			{
				if (flags & TASKS_FLAGS_UPDATE_NEWER)
				{
					if (std::tr2::sys::last_write_time(destFile) < srcFileTime)
						toUpdate = true;
				}
				else if ((flags & TASKS_FLAGS_UPDATE_KEEP_BOTH))
				{
					if ((std::tr2::sys::last_write_time(destFile) != srcFileTime) ||
						(std::tr2::sys::file_size(destFile) != srcFileSize))
					{
						Util::MD5Hash md5{ srcFile };
						if (GetNewFileName(destFile, md5))
							toAddNew = true;
					}
				}
				// else TASKS_FLAGS_UPDATE_DO_NOTHING
			}
			else
				toAddNew = true;

			if (toUpdate)
			{
				WriteLog(Log::LogLevel::Verbose, _T("Updating \"%s\""), srcFile.wstring().c_str());
				if (!m_testRun)
					std::tr2::sys::copy_file(srcFile, destFile, std::tr2::sys::copy_options::overwrite_existing);

				++updated;
			}
			else if (toAddNew)
			{
				WriteLog(Log::LogLevel::Verbose, _T("Adding \"%s\", source:\"%s\""), destFile.wstring().c_str(), srcFile.wstring().c_str());
				if (!m_testRun)
					std::tr2::sys::copy_file(srcFile, destFile);

				++added;
			}
			else
			{
				WriteLog(Log::LogLevel::Verbose, _T("Skipped \"%s\""), srcFile.wstring().c_str());
				++skipped;
			}
		}
	}
	WriteLog(Log::LogLevel::Info, _T("End replication. %d added, %d updated, %d skipped."), added, updated, skipped);
}

bool RepRunner::GetNewFileName(PathT& destFile, const Util::MD5Hash& md5)
{
	bool ret = true;
	// find a new filename
	StringT filename = destFile.stem();
	StringT ext = destFile.extension();
	int index = 1;

	do
	{
		Util::MD5Hash destMD5{ destFile };
		if (destMD5 == md5)
			return false;

		destFile.remove_filename();
		destFile /= filename + _T("(") + ToStringT(index++) + _T(")") + ext;
	} while (std::tr2::sys::exists(destFile));
	return ret;
}

void RepRunner::AddPath(const PathT& p, RepSource& repSource, int flags, bool matchExtension, const RegexT& filterRegex, int& fileCount)
{
	try
	{
		if (std::tr2::sys::is_regular_file(p))
		{
			bool add = false;

			if (matchExtension)
			{
				bool match = std::regex_match(p.extension().wstring(), filterRegex);
				if (((flags & TASKS_FLAGS_INCLUDE_FILTERS) && match) || ((flags & TASKS_FLAGS_EXCLUDE_FILTERS) && !match))
					add = true;
			}
			else
				add = true;

			if (add)
			{
				++fileCount;
				repSource.add(p);
				WriteLog(Log::LogLevel::Verbose, _T("[%d] \"%s\"."), fileCount, p.wstring().c_str());
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
	}
}

void RepRunner::WriteLog(Log::LogLevel level, LPCTSTR format, ...)
{
	TCHAR szMessage[2 * 1024];
	va_list args;
	va_start(args, format);
	_vstprintf_s(szMessage, _countof(szMessage), format, args);
	va_end(args);

	m_log.Write(level, szMessage);
	if (m_callback) m_callback(RunnerState::RUNNING, szMessage);
}

void RepRunner::UpdateTaskInDB(int taskID, Database::PropertyList& propList)
{
	StringT condition = TASKS_COL_TASKID;
	condition += _T("=") + ToStringT(taskID);

	Database::Table table{ m_db, TASKS_TABLE };
	table.Update(propList, condition);
}

RepSource RepRunner::GetSource(const PathT& path, int flags, bool matchExtension, const RegexT& filterRegex, int& fileCount)
{
	PathT srcPath{ path };
	RepSource source{ srcPath };

	WriteLog(Log::LogLevel::Info, _T("Discovering source directory [%s]..."), srcPath.wstring().c_str());
	if (flags & TASK_INCLUDE_SUBDIR)
	{
		RecursiveDirectoryIteratorT dit(srcPath);
		for (; dit != RecursiveDirectoryIteratorT(); ++dit)
		{
			if (m_abort) break;
			AddPath(dit->path(), source, flags, matchExtension, filterRegex, fileCount);
		}
	}
	else
	{
		DirectoryIteratorT dit(srcPath);
		for (; dit != DirectoryIteratorT(); ++dit)
		{
			if (m_abort) break;
			AddPath(dit->path(), source, flags, matchExtension, filterRegex, fileCount);
		}
	}
	return source;
}

void RepRunner::AddHistory(Database::PropertyList& propList)
{
	Database::Table table{ m_db, HISTORY_TABLE };
	table.Insert(propList);
}
