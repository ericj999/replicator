#include "stdafx.h"
#include <iostream>
#include "RepRunner.h"
#include "GlobDef.h"
#include "TaskRecord.h"
#include "Table.h"
#include "NamedPipe.h"

#define STR_LOG_FILENAME	_T("RepRunner.log")

RepRunner::RepRunner() :
m_taskID{ 0 }, m_verbose{ false }, m_testRun{ false }
{
	PathT logPath = Util::GetConfigPath();
	logPath /= STR_LOG_FILENAME;
	m_log.setPath(logPath);

	PathT dbPath{ Util::GetDatabasePath() };
	m_db.Connect(dbPath);
}

RepRunner::~RepRunner()
{
}

bool RepRunner::ParseCommandLine(int argc, _TCHAR* argv[])
{
	if (argc < 2)
		return false;

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == _T('-'))
		{
			switch (argv[i][1])
			{
			case _T('v'):
			case _T('V'):
				m_verbose = true;
				break;

			case _T('t'):
			case _T('T'):
				m_testRun = true;
				break;
			}
		}
		else
		{
			int arg = _wtoi(argv[i]);
			if (arg > 0)
				m_taskID = arg;
		}
	}

	if (m_taskID == 0)
		return false;

	return true;
}


void RepRunner::run()
{
	if (m_verbose)
		m_log.setLevel(Log::LogLevel::Verbose);

	IPC::PipeClient pipeClient{ STR_IPC_NAMED_PIPE_NAME, IPC::PipeDirection::both };

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
	// discovering....
	for (auto&& src : sources)
	{
		PathT srcPath(src);

		WriteLog(Log::LogLevel::Info, _T("Discovering source directory [%s]..."), srcPath.wstring().c_str());
		if (flags & TASK_INCLUDE_SUBDIR)
		{
			RecursiveDirectoryIteratorT dit(srcPath);
			for (; dit != RecursiveDirectoryIteratorT(); ++dit)
			{
				AddPath(dit->path(), paths, flags, matchExtension, filterRegex, fileCount);
			}
		}
		else
		{
			DirectoryIteratorT dit(srcPath);
			for (; dit != DirectoryIteratorT(); ++dit)
			{
				AddPath(dit->path(), paths, flags, matchExtension, filterRegex, fileCount);
			}
		}
	}
	WriteLog(Log::LogLevel::Info, _T("Discoverd total %d file(s)."), fileCount);

	// processing...
	PathT destination{ task.getDestination() };
	WriteLog(Log::LogLevel::Info, _T("Destination = \"%s\""), destination.wstring().c_str());

	int updated = 0, skipped = 0, added = 0;
	ReplicateNow(paths, destination, flags, added, updated, skipped);

	std::chrono::system_clock::duration dur = std::chrono::system_clock::now() - start;
	std::chrono::seconds runTime = std::chrono::duration_cast<std::chrono::seconds>(dur);

	StringT runTimeStr = Util::GetDurationString(runTime);
	StringT result = _T("Duration: ") + runTimeStr + _T(". ") + ToStringT(fileCount) + _T(" discovered, ") +
		ToStringT(added) + _T(" added, ") + ToStringT(updated) + _T(" updated, ") + ToStringT(skipped) + _T(" up-to-date.");

	propList.clear();
	propList.push_back(Database::Property(TASKS_COL_LASTRUNSTATUS, result));

	UpdateTaskInDB(m_taskID, propList);

}

void RepRunner::ReplicateNow(const std::vector<PathT>& paths, const PathT& destination, int flags, int& added, int& updated, int& skipped)
{
	for (auto&& srcFile : paths)
	{
		PathT destFile{ destination };
		destFile /= srcFile.relative_path();

		if (!std::tr2::sys::exists(destFile.parent_path()))
			std::tr2::sys::create_directories(destFile.parent_path());

		bool toUpdate = false, toAddNew = false;

		if (std::tr2::sys::exists(destFile))
		{
			if (std::tr2::sys::last_write_time(destFile) < std::tr2::sys::last_write_time(srcFile))
			{
				if (flags & TASKS_FLAGS_UPDATE_NEWER)
					toUpdate = true;
				else if ((flags & TASKS_FLAGS_UPDATE_KEEP_BOTH))
				{
					GetNewFileName(destFile);
					toAddNew = true;
				}
				// else TASKS_FLAGS_UPDATE_DO_NOTHING
			}
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
			WriteLog(Log::LogLevel::Verbose, _T("Adding \"%s\""), destFile.wstring().c_str());
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
	WriteLog(Log::LogLevel::Info, _T("End replication. %d added, %d updated, %d skipped."), added, updated, skipped);
}

void RepRunner::GetNewFileName(PathT& destFile)
{
	// find a new filename
	StringT filename = destFile.stem();
	StringT ext = destFile.extension();
	int index = 1;

	do
	{
		destFile.remove_filename();
		destFile /= filename + _T("-") + ToStringT(index++) + ext;
	} while (std::tr2::sys::exists(destFile));
}

void RepRunner::AddPath(const PathT& p, std::vector<PathT>& paths, int flags, bool matchExtension, const RegexT& filterRegex, int& fileCount)
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
				paths.push_back(p);
				WriteLog(Log::LogLevel::Verbose, _T("Add [%d] \"%s\"."), fileCount, p.wstring().c_str());
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
}

void RepRunner::UpdateTaskInDB(int taskID, Database::PropertyList& propList)
{
	StringT condition = TASKS_COL_TASKID;
	condition += _T("=") + ToStringT(taskID);

	Database::Table table{ m_db, TASKS_TABLE };
	table.Update(propList, condition);
}
