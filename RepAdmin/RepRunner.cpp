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
#include "Replicator.h"
#include "ShellFolder.h"
#include "FileTime.h"

#include "resource.h"

#define STR_LOG_FILENAME	_T("Replicate-")
#define STR_LOG_EXTENSION	_T(".log")

#define ENUM_COUNT 1	//50
#define SHELLITEM_REQUIRED_ATTRIBUTES (SFGAO_CANCOPY | SFGAO_STREAM)

RepRunner::RepRunner(int taskID, RunnerEventCallback callback /* = nullptr*/, bool verbose /*= false*/, bool testRun /*= false*/) :
	m_taskID{ taskID }, m_callback{ callback }, m_verbose{ verbose }, m_testRun{ testRun }, m_abort{ false }, m_isRunning{ false },
	m_fileCount{ 0 }, m_updated{ 0 }, m_skipped{ 0 }, m_added{ 0 }, m_flags{ 0 }, m_matchExtension{ 0 }
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
	CoInitialize(NULL);	// run in STA model since the portable device shell doesn't support MTA
	_This->Run();
	CoUninitialize();
}

void RepRunner::AsyncRun()
{
	m_thread = std::thread{ Thread, this };
}

void RepRunner::Run()
{
	m_fileCount = 0; m_updated = 0; m_skipped = 0; m_added = 0;

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
		std::vector<PathT> paths;

		m_flags = task.getFlags();

		StringT regexFilters = String::GetFiltersExp(task.getFilters());
		m_matchExtension = (m_flags & (TASKS_FLAGS_INCLUDE_FILTERS | TASKS_FLAGS_EXCLUDE_FILTERS)) && !regexFilters.empty();
		m_filterRegex.assign(regexFilters, std::regex::ECMAScript | std::regex::icase);

		WriteLog(Log::LogLevel::Info, _T("Flags = 0x%X"), m_flags);
		WriteLog(Log::LogLevel::Info, _T("Filters=\"%s\""), regexFilters.c_str());

		StringT destFolderFormat = task.getDestinationFolderFormat();

		if (m_flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
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

		m_srcPath = task.getSource();
		m_destination = task.getDestination();
		m_parsingSrc = task.getParsingSource();

		if (!m_parsingSrc.empty() && (m_parsingSrc[0] == _T(':')))
		{
			// run Shell APIs....
			ReplicateWithShell(m_parsingSrc, m_srcPath, m_destination);
		}
		else
		{
			// regular file system
			// discovering....
			std::vector<RepSource> repSources;
			repSources.push_back(GetSource(m_srcPath));

			if (!m_abort)
			{
				WriteLog(Log::LogLevel::Info, _T("Discoverd total %d file(s)."), m_fileCount);

				// replicating....
				WriteLog(Log::LogLevel::Info, _T("Destination = \"%s\""), m_destination.wstring().c_str());

				ReplicateNow(repSources, m_destination);
			}
		}
		// done
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

		cstr.Format(IDS_RESULT_FORMAT, (LPCTSTR) rstr, runTimeStr.c_str(), m_fileCount, m_added, m_updated, m_skipped);

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

void RepRunner::ReplicateNow(const std::vector<RepSource>& repSources, const PathT& destination)
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

			auto srcFileTime = std::experimental::filesystem::last_write_time(srcFile);
			auto srcFileSize = std::experimental::filesystem::file_size(srcFile);

			if (m_flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
			{
				std::time_t tt = decltype(srcFileTime)::clock::to_time_t(srcFileTime);
				std::tm ft;
				localtime_s(&ft, &tt);

				destFile /= m_pathFormatter.GetPath(&ft);
				destFile /= srcFile.filename();
			}
			else
			{
				if(m_flags & TASKS_FLAGS_DEST_START_FROM_ROOT)
					destFile /= srcFile.relative_path();
				else
					destFile /= src;
			}

			if (!std::experimental::filesystem::exists(destFile.parent_path()))
				std::experimental::filesystem::create_directories(destFile.parent_path());

			bool toUpdate = false, toAddNew = false;

			if (std::experimental::filesystem::exists(destFile))
			{
				if (m_flags & TASKS_FLAGS_UPDATE_NEWER)
				{
					if (std::experimental::filesystem::last_write_time(destFile) < srcFileTime)
						toUpdate = true;
				}
				else if (m_flags & TASKS_FLAGS_UPDATE_KEEP_BOTH)
				{
					if ((std::experimental::filesystem::last_write_time(destFile) != srcFileTime) ||
						(std::experimental::filesystem::file_size(destFile) != srcFileSize))
					{
						Util::MD5Hash md5{ srcFile };
						if (GetNewFileName(destFile, md5))
							toAddNew = true;
					}
				}
				else if (m_flags & TASKS_FLAGS_UPDATE_OVERWRITE)
				{
					toUpdate = true;
				}
				// else TASKS_FLAGS_UPDATE_DO_NOTHING
			}
			else
				toAddNew = true;

			if (toUpdate)
			{
				WriteLog(Log::LogLevel::Verbose, _T("Updating \"%s\""), srcFile.wstring().c_str());
				if (!m_testRun)
					std::experimental::filesystem::copy_file(srcFile, destFile, std::experimental::filesystem::copy_options::overwrite_existing);

				++m_updated;
			}
			else if (toAddNew)
			{
				WriteLog(Log::LogLevel::Verbose, _T("Adding \"%s\", source:\"%s\""), destFile.wstring().c_str(), srcFile.wstring().c_str());
				if (!m_testRun)
					std::experimental::filesystem::copy_file(srcFile, destFile);

				++m_added;
			}
			else
			{
				WriteLog(Log::LogLevel::Verbose, _T("Skipped \"%s\""), srcFile.wstring().c_str());
				++m_skipped;
			}
		}
	}
	WriteLog(Log::LogLevel::Info, _T("End replication. %d added, %d updated, %d skipped."), m_added, m_updated, m_skipped);
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
	} while (std::experimental::filesystem::exists(destFile));
	return ret;
}

void RepRunner::AddPath(const PathT& p, RepSource& repSource)
{
	try
	{
		if (std::experimental::filesystem::is_regular_file(p))
		{
			bool add = false;

			if (m_matchExtension)
			{
				bool match = std::regex_match(p.extension().wstring(), m_filterRegex);
				if (((m_flags & TASKS_FLAGS_INCLUDE_FILTERS) && match) || ((m_flags & TASKS_FLAGS_EXCLUDE_FILTERS) && !match))
					add = true;
			}
			else
				add = true;

			if (add)
			{
				++m_fileCount;
				repSource.add(p);
				WriteLog(Log::LogLevel::Verbose, _T("[%d] \"%s\"."), m_fileCount, p.wstring().c_str());
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

RepSource RepRunner::GetSource(const PathT& path)
{
	PathT srcPath{ path };
	RepSource source{ srcPath };

	WriteLog(Log::LogLevel::Info, _T("Discovering source directory [%s]..."), srcPath.wstring().c_str());
	if (m_flags & TASKS_FLAGS_INCLUDE_SUBDIR)
	{
		RecursiveDirectoryIteratorT dit(srcPath);
		for (; dit != RecursiveDirectoryIteratorT(); ++dit)
		{
			if (m_abort) break;
			AddPath(dit->path(), source);
		}
	}
	else
	{
		DirectoryIteratorT dit(srcPath);
		for (; dit != DirectoryIteratorT(); ++dit)
		{
			if (m_abort) break;
			AddPath(dit->path(), source);
		}
	}
	return source;
}

void RepRunner::AddHistory(Database::PropertyList& propList)
{
	if (theApp.getHistoryDays() > 0)
	{
		Database::Table table{ m_db, HISTORY_TABLE };
		table.Insert(propList);
	}
}

void RepRunner::ReplicateWithShell(const StringT& parsingSrc, const PathT& srcPath, const PathT& destination)
{
	ShellWrapper::ShellFolder folder{ parsingSrc };

	ReplicateWithShell2(folder, srcPath, destination);
}

void RepRunner::ReplicateWithShell2(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination)
{
	// get all the ID's first
	HRESULT hr = E_FAIL;
	ShellWrapper::EnumIDList enumIDList;
	ShellItemIDList folderList;
	ShellItemIDList fileList;

	SHCONTF scf = SHCONTF_NONFOLDERS;
	if (m_flags & TASKS_FLAGS_INCLUDE_SUBDIR)
		scf |= SHCONTF_FOLDERS;

	if (SUCCEEDED(hr = folder->EnumObjects(NULL, scf, &enumIDList)))
	{
		HRESULT getResult = E_FAIL;
		do
		{
			if (m_abort)
			{
				WriteLog(Log::LogLevel::Info, _T("Replication was aborted."));
				return;
			}

			ULONG got = 0;
			LPITEMIDLIST itemIdList = nullptr;
			getResult = enumIDList->Next(1, &itemIdList, &got);
			if ((getResult == S_OK) && (got == 1))
			{
				ShellWrapper::ShellItem shellItem;
				if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), itemIdList, IID_IShellItem2, (void**)&shellItem)))
				{
					if (shellItem.IsFolder())
						folderList.push_back(itemIdList);
					else
						fileList.push_back(itemIdList);
				}
			}
		} while (getResult == S_OK);
	}
	// process all file objects
	if(!fileList.empty())
		ProcessFiles(folder, srcPath, destination, fileList);

	// process all folder objects
	if(!folderList.empty())
		ProcessFolders(folder, srcPath, destination, folderList);
}

void RepRunner::ProcessFiles(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList)
{
	HRESULT hr;

	for (auto it = shellItemIdList.begin(); it != shellItemIdList.end(); ++it)
	{
		ShellWrapper::ShellItem shellItem;
		if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), *it, IID_IShellItem2, (void**)&shellItem)))
		{
			PathT filePath{ srcPath };

			filePath /= shellItem.GetName();

			// extension check
			bool matchExt = false;
			if (m_matchExtension)
			{
				bool match = std::regex_match(filePath.extension().wstring(), m_filterRegex);
				if (((m_flags & TASKS_FLAGS_INCLUDE_FILTERS) && match) || ((m_flags & TASKS_FLAGS_EXCLUDE_FILTERS) && !match))
					matchExt = true;
			}
			else
				matchExt = true;

			// condition check
			if (matchExt)
			{
				++m_fileCount;
				WriteLog(Log::LogLevel::Verbose, _T("[%d] \"%s\"."), m_fileCount, filePath.wstring().c_str());

				FILETIME srcFileTime = { 0 };
				UINT64 srcFileSize = 0;

				shellItem->GetUInt64(PKEY_Size, &srcFileSize);
				if (SUCCEEDED(hr = shellItem->GetFileTime(PKEY_DateModified, &srcFileTime)))
				{
					PathT destFile{ destination };

					if (m_flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
					{
						std::tm ft;
						FILETIME localFT;
						SYSTEMTIME st;

						FileTimeToLocalFileTime(&srcFileTime, &localFT);
						FileTimeToSystemTime(&localFT, &st);

						ft.tm_year = st.wYear - 1900;
						ft.tm_mon = st.wMonth;
						ft.tm_mday = st.wDay;
						ft.tm_hour = st.wHour;
						ft.tm_min = st.wMinute;
						ft.tm_sec = st.wSecond;

						destFile /= m_pathFormatter.GetPath(&ft);
						destFile /= shellItem.GetName();
					}
					else
					{
						if (m_flags & TASKS_FLAGS_DEST_START_FROM_ROOT)
							destFile /= filePath.relative_path();
						else
						{
							std::wstring src{ filePath.wstring().c_str() };
							destFile /= src.substr(m_srcPath.wstring().length());
							destFile /= shellItem.GetName();
						}
					}

					if (!std::experimental::filesystem::exists(destFile.parent_path()))
						std::experimental::filesystem::create_directories(destFile.parent_path());

					bool toUpdate = false, toAddNew = false;

					if (std::experimental::filesystem::exists(destFile))
					{
						FileTime destFileTime{ destFile };

						if (m_flags & TASKS_FLAGS_UPDATE_NEWER)
						{
							if (CompareFileTime(&destFileTime.getModifiedTime(), &srcFileTime) != 0)
								toUpdate = true;
						}
						else if (m_flags & TASKS_FLAGS_UPDATE_KEEP_BOTH)
						{
							if ((CompareFileTime(&destFileTime.getModifiedTime(), &srcFileTime) != 0) ||
								(std::experimental::filesystem::file_size(destFile) != srcFileSize))
							{
								Util::MD5Hash md5{ shellItem };
								if (GetNewFileName(destFile, md5))
									toAddNew = true;
							}
						}
						else if (m_flags & TASKS_FLAGS_UPDATE_OVERWRITE)
						{
							toUpdate = true;
						}
						// else TASKS_FLAGS_UPDATE_DO_NOTHING
					}
					else
						toAddNew = true;

					if (toUpdate)
					{
						WriteLog(Log::LogLevel::Verbose, _T("Updating \"%s\""), filePath.wstring().c_str());
						if (!m_testRun)
							Util::CopyStreamToFile(shellItem, destFile.wstring());

						++m_updated;
					}
					else if (toAddNew)
					{
						WriteLog(Log::LogLevel::Verbose, _T("Adding \"%s\", source:\"%s\""), destFile.wstring().c_str(), filePath.wstring().c_str());
						if (!m_testRun)
							Util::CopyStreamToFile(shellItem, destFile.wstring());

						++m_added;
					}
					else
					{
						WriteLog(Log::LogLevel::Verbose, _T("Skipped \"%s\""), filePath.wstring().c_str());
						++m_skipped;
					}
				}
			}
		}
	}
}

void RepRunner::ProcessFolders(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList)
{
	HRESULT hr;

	for (auto it = shellItemIdList.begin(); it != shellItemIdList.end(); ++it)
	{
		ShellWrapper::ShellItem shellItem;
		if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), *it, IID_IShellItem2, (void**)&shellItem)))
		{
			PathT filePath{ srcPath };

			filePath /= shellItem.GetName();
			ShellWrapper::ShellFolder childFolder;

			if (SUCCEEDED(hr = folder->BindToObject(*it, nullptr, IID_IShellFolder, (void**)&childFolder)))
			{
				ReplicateWithShell2(childFolder, filePath.wstring().c_str(), destination);
			}
		}
	}
}

void RepRunner::ReplicateWithShell(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination)
{

	HRESULT hr = E_FAIL;
	ShellWrapper::EnumIDList enumIDList;

	SHCONTF scf = SHCONTF_NONFOLDERS;
	if (m_flags & TASKS_FLAGS_INCLUDE_SUBDIR)
		scf |= SHCONTF_FOLDERS;

	if (SUCCEEDED(hr = folder->EnumObjects(NULL, scf, &enumIDList)))
	{
		HRESULT getResult = E_FAIL;
		do
		{
			if (m_abort)
			{
				WriteLog(Log::LogLevel::Info, _T("Replication was aborted."));
				return;
			}
			ULONG get = ENUM_COUNT, got = 0;
			ComMemArray<LPITEMIDLIST, ENUM_COUNT> folderItemIdList;

			getResult = enumIDList->Next(get, folderItemIdList.data(), &got);
			for (ULONG item = 0; item < got; ++item)
			{
				ShellWrapper::ShellItem shellItem;
				if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), folderItemIdList[item], IID_IShellItem2, (void**)&shellItem)))
				{
					PathT filePath{ srcPath };
					
					filePath /= shellItem.GetName();

					if (shellItem.IsFolder())
					{
						ShellWrapper::ShellFolder childFolder;

						if (SUCCEEDED(hr = folder->BindToObject(folderItemIdList[item], nullptr, IID_IShellFolder, (void**)&childFolder)))
							ReplicateWithShell(childFolder, filePath.wstring().c_str(), destination);
					}
					else
					{
						// extension check
						bool matchExt = false;
						if (m_matchExtension)
						{
							bool match = std::regex_match(filePath.extension().wstring(), m_filterRegex);
							if (((m_flags & TASKS_FLAGS_INCLUDE_FILTERS) && match) || ((m_flags & TASKS_FLAGS_EXCLUDE_FILTERS) && !match))
								matchExt = true;
						}
						else
							matchExt = true;

						// condition check
						if (matchExt)
						{
							++m_fileCount;
							WriteLog(Log::LogLevel::Verbose, _T("[%d] \"%s\"."), m_fileCount, filePath.wstring().c_str());

							FILETIME srcFileTime = { 0 };
							UINT64 srcFileSize = 0;
							
							shellItem->GetUInt64(PKEY_Size, &srcFileSize);
							if (SUCCEEDED(hr = shellItem->GetFileTime(PKEY_DateModified, &srcFileTime)))
							{
								PathT destFile{ destination };

								if (m_flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
								{
									std::tm ft;
									FILETIME localFT;
									SYSTEMTIME st;

									FileTimeToLocalFileTime(&srcFileTime, &localFT);
									FileTimeToSystemTime(&localFT, &st);

									ft.tm_year = st.wYear - 1900;
									ft.tm_mon = st.wMonth;
									ft.tm_mday = st.wDay;
									ft.tm_hour = st.wHour;
									ft.tm_min = st.wMinute;
									ft.tm_sec = st.wSecond;

									destFile /= m_pathFormatter.GetPath(&ft);
									destFile /= shellItem.GetName();
								}
								else
								{
									if (m_flags & TASKS_FLAGS_DEST_START_FROM_ROOT)
										destFile /= filePath.relative_path();
									else
									{
										std::wstring src{ filePath.wstring().c_str() };
										destFile /= src.substr(m_srcPath.wstring().length());
										destFile /= shellItem.GetName();
									}
								}

								if (!std::experimental::filesystem::exists(destFile.parent_path()))
									std::experimental::filesystem::create_directories(destFile.parent_path());

								bool toUpdate = false, toAddNew = false;

								if (std::experimental::filesystem::exists(destFile))
								{
									FileTime destFileTime{ destFile };

									if (m_flags & TASKS_FLAGS_UPDATE_NEWER)
									{
										if (CompareFileTime(&destFileTime.getModifiedTime(), &srcFileTime) != 0)
											toUpdate = true;
									}
									else if (m_flags & TASKS_FLAGS_UPDATE_KEEP_BOTH)
									{
										if ((CompareFileTime(&destFileTime.getModifiedTime(), &srcFileTime) != 0) ||
											(std::experimental::filesystem::file_size(destFile) != srcFileSize))
										{
											Util::MD5Hash md5{ shellItem };
											if (GetNewFileName(destFile, md5))
												toAddNew = true;
										}
									}
									else if (m_flags & TASKS_FLAGS_UPDATE_OVERWRITE)
									{
										toUpdate = true;
									}
									// else TASKS_FLAGS_UPDATE_DO_NOTHING
								}
								else
									toAddNew = true;

								if (toUpdate)
								{
									WriteLog(Log::LogLevel::Verbose, _T("Updating \"%s\""), filePath.wstring().c_str());
									if (!m_testRun)
										Util::CopyStreamToFile(shellItem,  destFile.wstring());

									++m_updated;
								}
								else if (toAddNew)
								{
									WriteLog(Log::LogLevel::Verbose, _T("Adding \"%s\", source:\"%s\""), destFile.wstring().c_str(), filePath.wstring().c_str());
									if (!m_testRun)
										Util::CopyStreamToFile(shellItem, destFile.wstring());

									++m_added;
								}
								else
								{
									WriteLog(Log::LogLevel::Verbose, _T("Skipped \"%s\""), filePath.wstring().c_str());
									++m_skipped;
								}
							}
						}
					}
				}
			}
		} while (getResult == S_OK);
	}

}

