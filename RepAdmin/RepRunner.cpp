#include "stdafx.h"
#include <iostream>
#include <ctime>
#include <locale>
#include <codecvt>
#include <map>
#include "RepRunner.h"
#include "GlobDef.h"
#include "TaskRecord.h"
#include "Table.h"
#include "RepSource.h"
#include "Replicator.h"
#include "ShellFolder.h"
#include "FileTime.h"
#include "WPDPortableDevice.h"
#include "WPDPortableDeviceContent.h"
#include "WPDPortableDeviceProperties.h"
#include "util.h"
#include "LocaleResources.h"

#include "resource.h"

#define NUM_OBJECTS_TO_REQUEST	10

RepRunner::RepRunner(int taskID, RunnerEventCallback callback /* = nullptr*/, bool verbose /*= false*/, bool testRun /*= false*/) :
	m_taskID{ taskID }, m_callback{ callback }, m_verbose{ verbose }, m_testRun{ testRun }, m_abort{ false }, m_isRunning{ false },
	m_fileCount{ 0 }, m_updated{ 0 }, m_skipped{ 0 }, m_added{ 0 }, m_flags{ 0 }, m_matchExtension{ 0 }, m_doCallBack{ true }
{
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
	std::chrono::system_clock::time_point start{ std::chrono::system_clock::now() };
	StringT startTimeStr = Util::GetIsoTimeString(std::chrono::system_clock::to_time_t(start));
	StringT endTimeStr;
	try
	{
		WriteLog(Log::LogLevel::Verbose, _T("==============================="));
		WriteLog(Log::LogLevel::Verbose, _T("Begin replication task ID %d..."), m_taskID);
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

		WriteLog(Log::LogLevel::Verbose, _T("Flags = 0x%X"), m_flags);
		WriteLog(Log::LogLevel::Verbose, _T("Filters=\"%s\""), regexFilters.c_str());

		StringT destFolderFormat = task.getDestinationFolderFormat();

		if (m_flags & TASKS_FLAGS_DEST_GROUP_BY_DATE)
		{
			if (destFolderFormat.empty())
			{
				throw std::runtime_error( EXCEPSTR_DEST_FOLDER_FORMAT_EMPTY );
			}
			m_pathFormatter.SetFormat(destFolderFormat);
		}

		m_srcPath = task.getSource();
		m_destination = task.getDestination();
		m_parsingSrc = task.getParsingSource();
		m_parsingDestination = task.getParsingDestination();

		if (!m_parsingSrc.empty() && (m_parsingSrc[0] == _T(':')))
		{
			// run Shell APIs....
			ReplicateStreamToFile(m_parsingSrc, m_srcPath, m_parsingDestination);	// m_destination);
		}
		else if (!m_parsingDestination.empty() && (m_parsingDestination[0] == _T(':')))
		{
//			ReplicateFileToStream(m_parsingSrc, m_destination, m_parsingDestination);
			ReplicateFilesToPortableDevice(m_parsingSrc, m_destination, m_parsingDestination);
		}
		else
		{
			// regular file system
			// discovering....
			std::vector<RepSource> repSources;
			repSources.push_back(GetSource(m_srcPath));

			if (!m_abort)
			{
				WriteLog(Log::LogLevel::Verbose, _T("Discoverd total %d file(s)."), m_fileCount);

				// replicating....
				WriteLog(Log::LogLevel::Verbose, _T("Destination = \"%s\""), m_destination.wstring().c_str());

				ReplicateNow(repSources, m_destination);
			}
		}
		// done
		std::chrono::system_clock::time_point endTime{ std::chrono::system_clock::now() };

		propList.clear();

		CString cstr, rstr;
		std::chrono::system_clock::duration dur = endTime - start;
		std::chrono::seconds runTime = std::chrono::duration_cast<std::chrono::seconds>(dur);
		StringT runTimeStr = GetDurationString(runTime);
		endTimeStr = Util::GetIsoTimeString(std::chrono::system_clock::to_time_t(endTime));

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

		int failed = m_fileCount - m_added - m_updated - m_skipped;
		if (failed > 0)
		{
			cstr.Format(IDS_NOT_VERIFIED, failed);
			result += (LPCTSTR)cstr;
		}

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
		WriteLog(Log::LogLevel::Error, result.c_str());

		std::wstring locResult = GetLocalizedString(e.what());
		if (!locResult.empty())
			result = locResult;

		Database::PropertyList propList;
		propList.push_back(Database::Property(TASKS_COL_LASTRUNSTATUS, result));
		UpdateTaskInDB(m_taskID, propList);

		std::chrono::system_clock::time_point endTime{ std::chrono::system_clock::now() };
		endTimeStr = Util::GetIsoTimeString(std::chrono::system_clock::to_time_t(endTime));

		propList.clear();
		propList.push_back(Database::Property(HISTORY_COL_TASKID, m_taskID));
		propList.push_back(Database::Property(HISTORY_COL_START_TIME, startTimeStr));
		propList.push_back(Database::Property(HISTORY_COL_END_TIME, endTimeStr));
		propList.push_back(Database::Property(HISTORY_COL_RESULT, result));
		AddHistory(propList);
	}
	if (m_doCallBack && m_callback) m_callback(RunnerState::STOP, result.c_str());
	m_isRunning = false;
	WriteLog(Log::LogLevel::Verbose, _T("End replication task ID %d."), m_taskID);
	WriteLog(Log::LogLevel::Verbose, _T("==============================="));
}

bool RepRunner::MatchedExtension(const std::wstring& ext)
{
	bool matchExt = false;

	if (m_matchExtension)
	{
		bool match = std::regex_match(ext, m_filterRegex);
		if (((m_flags & TASKS_FLAGS_INCLUDE_FILTERS) && match) || ((m_flags & TASKS_FLAGS_EXCLUDE_FILTERS) && !match))
			matchExt = true;
	}
	else
		matchExt = true;

	return matchExt;
}

void RepRunner::ReplicateNow(const std::vector<RepSource>& repSources, const PathT& destination)
{
	for (auto&& source : repSources)
	{
		for (auto&& src : source.getChildren())
		{
			if (m_abort)
			{
				WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str());
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
				WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::updating).c_str(), destFile.wstring().c_str());
				if (!m_testRun)
				{
					if (std::experimental::filesystem::copy_file(srcFile, destFile, std::experimental::filesystem::copy_options::overwrite_existing))
						++m_updated;
				}
				else
					++m_updated;
			}
			else if (toAddNew)
			{
				WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::adding).c_str(), destFile.wstring().c_str(), srcFile.wstring().c_str());
				if (!m_testRun)
				{
					if(std::experimental::filesystem::copy_file(srcFile, destFile))
						++m_added;
				}
				else
					++m_added;
			}
			else
			{
				WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::skipped).c_str(), srcFile.wstring().c_str());
				++m_skipped;
			}
		}
	}
	WriteLog(Log::LogLevel::Verbose, _T("End replication. %d added, %d updated, %d skipped."), m_added, m_updated, m_skipped);
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
			if (MatchedExtension(p.extension().wstring()))
			{
				++m_fileCount;
				repSource.add(p);
				WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::found).c_str(), m_fileCount, p.wstring().c_str());
			}
		}
	}
	catch (std::exception& e)
	{
		Log::logger.error(StringT(_T("Exception caught: ")) + String::StringToStringT(e.what()));
	}
}

void RepRunner::WriteLog(Log::LogLevel level, LPCTSTR format, ...)
{
	TCHAR szMessage[2 * 1024];
	va_list args;
	va_start(args, format);
	_vstprintf_s(szMessage, _countof(szMessage), format, args);
	va_end(args);

	Log::logger.Write(level, szMessage);
	if (m_doCallBack && m_callback && (level <= Log::LogLevel::Info)) m_callback(RunnerState::RUNNING, szMessage);
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

	WriteLog(Log::LogLevel::Verbose, _T("Discovering source directory [%s]..."), srcPath.wstring().c_str());

	if (m_flags & TASKS_FLAGS_INCLUDE_SUBDIR)
	{
		RecursiveDirectoryIteratorT dit(srcPath);
		for (; dit != RecursiveDirectoryIteratorT(); ++dit)
		{
			if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); break; }
			AddPath(dit->path(), source);
		}
	}
	else
	{
		DirectoryIteratorT dit(srcPath);
		for (; dit != DirectoryIteratorT(); ++dit)
		{
			if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); break; }
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

void RepRunner::ReplicateStreamToFile(const StringT& parsingSrc, const PathT& srcPath, const PathT& destination)
{
	ShellWrapper::ShellFolder folder{ parsingSrc };

	ReplicateStreamToFile(folder, srcPath, destination);
}

void RepRunner::ReplicateStreamToFile(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination)
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
			if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

			ULONG got = 0;
			LPITEMIDLIST itemIdList = nullptr;
			getResult = enumIDList->Next(1, &itemIdList, &got);
			if ((getResult == S_OK) && (got == 1))
			{
				ShellWrapper::ShellItem2 shellItem;
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
		ProcessStreamFiles(folder, srcPath, destination, fileList);

	// process all folder objects
	if(!folderList.empty())
		ProcessStreamFolders(folder, srcPath, destination, folderList);
}

void RepRunner::ProcessStreamFiles(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList)
{
	HRESULT hr;

	for (auto it = shellItemIdList.begin(); it != shellItemIdList.end(); ++it)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		ShellWrapper::ShellItem2 shellItem;
		if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), *it, IID_IShellItem2, (void**)&shellItem)))
		{
			PathT filePath{ srcPath };
			filePath /= shellItem.GetName();

			// condition check
			if (MatchedExtension(filePath.extension().wstring()))
			{
				++m_fileCount;
				WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::found).c_str(), m_fileCount, filePath.wstring().c_str());

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

						memset(&ft, 0, sizeof(ft));
						ft.tm_year = st.wYear - 1900;
						ft.tm_mon = st.wMonth - 1;
						ft.tm_mday = st.wDay;
						ft.tm_hour = st.wHour;
						ft.tm_min = st.wMinute;
						ft.tm_sec = st.wSecond;
						ft.tm_wday = st.wDayOfWeek;

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
							if (CompareFileTime(destFileTime.getModifiedTime(), &srcFileTime) < 0)
								toUpdate = true;
						}
						else if (m_flags & TASKS_FLAGS_UPDATE_KEEP_BOTH)
						{
							if ((CompareFileTime(destFileTime.getModifiedTime(), &srcFileTime) != 0) ||
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
						WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::updating).c_str(), filePath.wstring().c_str());
						if (!m_testRun)
						{
							if(Util::CopyStreamToFile(shellItem, destFile.wstring()))
								++m_updated;
							else
								WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToUpdate).c_str(), filePath.wstring().c_str());
						}
						else
							++m_updated;
					}
					else if (toAddNew)
					{
						WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::adding).c_str(), destFile.wstring().c_str(), filePath.wstring().c_str());
						if (!m_testRun)
						{
							if(Util::CopyStreamToFile(shellItem, destFile.wstring()))
								++m_added;
							else
								WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToAdd).c_str(), destFile.wstring().c_str());
						}
						else
							++m_added;
					}
					else
					{
						WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::skipped).c_str(), filePath.wstring().c_str());
						++m_skipped;
					}
				}
				else
				{
					Log::logger.error(StringT(_T("Failed to get filetime. Code:")) + ToStringT(hr));
				}
			}
		}
	}
}

void RepRunner::ProcessStreamFolders(ShellWrapper::ShellFolder& folder, const PathT& srcPath, const PathT& destination, const ShellItemIDList& shellItemIdList)
{
	HRESULT hr;

	for (auto it = shellItemIdList.begin(); it != shellItemIdList.end(); ++it)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		ShellWrapper::ShellItem2 shellItem;
		if (SUCCEEDED(hr = SHCreateItemWithParent(nullptr, folder.Get(), *it, IID_IShellItem2, (void**)&shellItem)))
		{
			PathT filePath{ srcPath };
			PathT newDestPath{ destination };

			filePath /= shellItem.GetName();
			if (!(m_flags & (TASKS_FLAGS_DEST_GROUP_BY_DATE | TASKS_FLAGS_DEST_START_FROM_ROOT)))
			{
				newDestPath /= shellItem.GetName();
			}

			ShellWrapper::ShellFolder childFolder;

			if (SUCCEEDED(hr = folder->BindToObject(*it, nullptr, IID_IShellFolder, (void**)&childFolder)))
			{
				ReplicateStreamToFile(childFolder, filePath.wstring().c_str(), newDestPath);
			}
		}
	}
}

void RepRunner::ReplicateFileToStream(const PathT& srcPath, const PathT& destination, const PathT& parsingDest)
{
	ShellWrapper::ShellFolder folder{ parsingDest };

	ReplicateFileToStream(srcPath, folder, destination);
}

void RepRunner::ReplicateFileToStream(const PathT& srcPath, ShellWrapper::ShellFolder& folder, const PathT& destination)
{
	bool includeSubFolder = (m_flags & TASKS_FLAGS_INCLUDE_SUBDIR) ? true : false;
	std::vector<PathT> fileList;
	std::vector<PathT> folderList;

	DirectoryIteratorT dit(srcPath);
	for (; dit != DirectoryIteratorT(); ++dit)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		try
		{
			if (std::experimental::filesystem::is_regular_file(dit->path()))
			{
				if (MatchedExtension(dit->path().extension().wstring()))
				{
					++m_fileCount;
					fileList.push_back(dit->path());
					WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::found).c_str(), m_fileCount, dit->path().wstring().c_str());
				}
			}
			else if (includeSubFolder && std::experimental::filesystem::is_directory(dit->path()))
			{
				folderList.push_back(dit->path());
			}
		}
		catch (std::exception& e)
		{
			Log::logger.error(StringT(_T("Exception caught: ")) + String::StringToStringT(e.what()));
		}
	}

	if (!fileList.empty())
	{
		ProcessFiles(fileList, folder, destination);
	}

	if (!folderList.empty())
	{
		ProcessFolders(folderList, folder, destination);
	}
}

// fileList: the list of child file items
// folder: the current destination folder object
void RepRunner::ProcessFiles(const std::vector<PathT>& fileList, ShellWrapper::ShellFolder& folder, const PathT& destination)
{
	ShellWrapper::ShellItem folderItem;

	HRESULT hr = SHGetItemFromObject(folder.Get(), IID_IShellItem, (void**)&folderItem);
	if (FAILED(hr))
	{
		Log::logger.error(StringT(_T("Failed to retrieve the shell item \"")) + destination.wstring() + StringT(_T("\". Code:")) + ToStringT(hr));
		return;
	}

	for (auto srcFile : fileList)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		bool add = false, update = false;
		std::wstring filename = srcFile.filename().wstring();

		ShellWrapper::ShellItem2 item = folder.OpenFileItem(filename);

		if (item.isValid())
		{
			HRESULT hr = E_FAIL;
			UINT64 srcFileSize = 0;

			item->GetUInt64(PKEY_Size, &srcFileSize);

			FileTime destFileTime{ item };
			FileTime srcFileTime{ srcFile.wstring() };

			if (srcFileTime.isValid() && destFileTime.isValid())
			{
				if (CompareFileTime(srcFileTime.getModifiedTime(), destFileTime.getModifiedTime()) > 0)
					update = true;
			}
			else
				update = true;
		}
		else
			add = true;

		PathT destFilePath{ destination };
		destFilePath /= filename;

		if (add)
		{
			WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::adding).c_str(), destFilePath.wstring().c_str(), srcFile.wstring().c_str());

			if (!m_testRun)
			{
				if (Util::CopyFileToFolder(srcFile, folderItem))
					++m_added;
				else
					WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToAdd).c_str(), destFilePath.wstring().c_str());
			}
			else
				++m_added;
		}
		else if (update)
		{
			WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::updating).c_str(), destFilePath.wstring().c_str());
			if (!m_testRun)
			{
				if (Util::CopyFileToFolder(srcFile, folderItem))
					++m_updated;
				else
					WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToUpdate).c_str(), destFilePath.wstring().c_str());
			}
			else
				++m_updated;
		}
		else
		{
			WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::skipped).c_str(), srcFile.wstring().c_str());
			++m_skipped;
		}
	}
}

// folderList: the list of subfolders 
// folder: the current destination folder object
void RepRunner::ProcessFolders(const std::vector<PathT>& folderList, ShellWrapper::ShellFolder& folder, const PathT& destination)
{
	for (auto folderPath : folderList)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		ShellWrapper::ShellFolder subFolder = folder.CreateSubFolder(folderPath.filename().wstring());

		PathT subPath{ destination };
		subPath /= folderPath.filename().wstring();

		ReplicateFileToStream(folderPath, subFolder, subPath);
	}
}

const std::wstring g_portableDevicePrefix{ L"\\\\?\\" };

void RepRunner::ReplicateFilesToPortableDevice(const PathT& srcPath, const PathT& destination, const PathT& parsingDest)
{
	std::vector<std::wstring> paths = Util::ParseParsingPath(parsingDest);
	if ((paths.size() < 3) || paths[0].empty() || (paths[0][0] != L':') || (paths[1].compare(0, g_portableDevicePrefix.length(), g_portableDevicePrefix) != 0))
	{
		throw new std::runtime_error("Invalid destination path.");
	}

	WPD::PortableDevice device{ paths[1] };

	WPD::PortableDeviceContent deviceContent;

	HRESULT hr = device->Content(&deviceContent);
	if (SUCCEEDED(hr))
	{
		ComInterface<IPortableDevicePropVariantCollection> objectIDs;
		ComInterface<IPortableDevicePropVariantCollection> persistentUniqueIDs{ CLSID_PortableDevicePropVariantCollection };
		ComPropVariant persistentId{ paths[paths.size() - 1] };

		hr = persistentUniqueIDs->Add(persistentId.Get());
		if (SUCCEEDED(hr))
		{
			hr = deviceContent->GetObjectIDsFromPersistentUniqueIDs(persistentUniqueIDs.Get(), &objectIDs);
			if (SUCCEEDED(hr))
			{
				DWORD count = 0;
				hr = objectIDs->GetCount(&count);
				if (SUCCEEDED(hr) && (count == 1))
				{
					ComPropVariant objectId;
					hr = objectIDs->GetAt(0, objectId.Get());
					if (SUCCEEDED(hr))
					{
						ReplicateFilesToPortableDevice(deviceContent, srcPath, objectId.GetStringValue(), destination);
					}
					else
					{
						Log::logger.error(StringT(_T("Failed to get the object ID value. Code:")) + ToStringT(hr));
					}
				}
				else
				{
					Log::logger.error(StringT(_T("Failed to retrieve the object ID. Count:")) + ToStringT(count) + StringT(_T(" Code:")) + ToStringT(hr));
				}
			}
			else
			{
				Log::logger.error(StringT(_T("Failed to retrieve the object ID. Code:")) + ToStringT(hr));
			}
		}
		else
		{
			Log::logger.error(StringT(_T("Failed to create property collection. Code")) + ToStringT(hr));
		}
	}
	else
	{
		Log::logger.error(StringT(_T("Failed to open portable device. Code:")) + ToStringT(hr));
	}
}

void RepRunner::ReplicateFilesToPortableDevice(WPD::PortableDeviceContent& deviceContent, const PathT& srcPath, const std::wstring& destObjId, const PathT& destination)
{
	bool includeSubFolder = (m_flags & TASKS_FLAGS_INCLUDE_SUBDIR) ? true : false;
	std::vector<PathT> fileList;
	std::vector<PathT> folderList;

	// discover all files and folder from the source
	DirectoryIteratorT dit(srcPath);
	for (; dit != DirectoryIteratorT(); ++dit)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		try
		{
			if (std::experimental::filesystem::is_regular_file(dit->path()))
			{
				if (MatchedExtension(dit->path().extension().wstring()))
				{
					++m_fileCount;
					fileList.push_back(dit->path());
					WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::found).c_str(), m_fileCount, dit->path().wstring().c_str());
				}
			}
			else if (includeSubFolder && std::experimental::filesystem::is_directory(dit->path()))
			{
				folderList.push_back(dit->path());
			}
		}
		catch (std::exception& e)
		{
			Log::logger.error(StringT(_T("Exception caught:")) + String::StringToStringT(e.what()).c_str());
		}
	}

	// discover all files in destination
	ComInterface<IEnumPortableDeviceObjectIDs>  enumObjectIDs;
	WPD::PortableDeviceProperties properties;

	WPD::PortableDeviceItemMap destFileMap;
	std::map<std::wstring, std::wstring> destFolderMap;

	HRESULT hr = E_FAIL;

	hr = deviceContent->Properties(&properties);
	if (SUCCEEDED(hr))
	{
		hr = deviceContent->EnumObjects(0, destObjId.c_str(), nullptr, &enumObjectIDs);
		while (hr == S_OK)
		{
			DWORD  numFetched = 0;
			PWSTR  objectIDArray[NUM_OBJECTS_TO_REQUEST] = { 0 };
			hr = enumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST, objectIDArray, &numFetched);
			if (SUCCEEDED(hr))
			{
				for (DWORD index = 0; (index < numFetched) && (objectIDArray[index] != nullptr); index++)
				{
					WPD::PortableDeviceItem item;
					if (SUCCEEDED(properties.GetItemWithProperties(objectIDArray[index], item)))
					{
						if (IsEqualGUID(item.GetContentType(), WPD_CONTENT_TYPE_FOLDER))
						{
							if(includeSubFolder)
								destFolderMap.insert(std::pair<std::wstring, std::wstring>(item.GetName(), item.GetObjId()));
						}
						else
						{
							destFileMap.insert(std::pair<std::wstring, WPD::PortableDeviceItem>(item.GetName(), item));
						}
					}
					CoTaskMemFree(objectIDArray[index]);
					objectIDArray[index] = nullptr;
				}
			}
		}
	}

	// process
	if (!fileList.empty())
	{
		ProcessFiles(deviceContent, fileList, destObjId, destination, destFileMap);
	}

	if (!folderList.empty())
	{
		ProcessFolders(deviceContent, folderList, destObjId, destination, destFolderMap);
	}
}

void RepRunner::ProcessFiles(WPD::PortableDeviceContent& deviceContent, const std::vector<PathT>& srcFileList, const std::wstring& currentFolderObjId,
	const PathT& currentDestPath, const WPD::PortableDeviceItemMap& destFileMap)
{
	WriteLog(Log::LogLevel::Verbose, L"source count = %d, destination count = %d", srcFileList.size(), destFileMap.size());
	for (auto srcFile : srcFileList)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		bool add = false, update = false;
		auto destFile = destFileMap.find(srcFile.filename());
		if (destFile != destFileMap.end())
		{
			FileTime srcFileTime{ srcFile };

			// do we need to update if different?
			if (CompareFileTime(srcFileTime.getModifiedTime(), destFile->second.GetModifiedTime()) > 0)
				update = true;
		}
		else
			add = true;

		PathT destFilePath{ currentDestPath };
		destFilePath /= srcFile.filename();

		if (update)
		{
			WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::updating).c_str(), destFilePath.wstring().c_str());
			if (m_testRun)
			{
				HRESULT hr = deviceContent.UpdateFile(srcFile, currentFolderObjId, destFile->second.GetObjId());
				if (SUCCEEDED(hr))
				{
					++m_updated;
				}
				else
				{
					WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToUpdate).c_str(), destFilePath.wstring().c_str());
					WriteLog(Log::LogLevel::Verbose, L"Error code: %x, src = %s", hr, srcFile.c_str());
				}
			}
			else
				++m_updated;
		}
		else if(add)
		{
			WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::adding).c_str(), destFilePath.wstring().c_str(), srcFile.wstring().c_str());
			if (!m_testRun)
			{
				HRESULT hr = deviceContent.TransferFile(srcFile, currentFolderObjId);
				if (SUCCEEDED(hr))
				{
					++m_added;
				}
				else
				{
					WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToAdd).c_str(), destFilePath.wstring().c_str());
					WriteLog(Log::LogLevel::Verbose, L"Error code: %x, src = %s", hr, srcFile.c_str());
				}
			}
			else
				++m_added;
		}
		else
		{
			WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::skipped).c_str(), srcFile.wstring().c_str());
			++m_skipped;
		}
	}

}
void RepRunner::ProcessFolders(WPD::PortableDeviceContent& deviceContent, const std::vector<PathT>& srcFolderList, const std::wstring& currentFolderObjId,
	const PathT& currentDestPath, const std::map<std::wstring, std::wstring>& destFolderMap)
{
	for (auto srcfolder : srcFolderList)
	{
		if (m_abort) { WriteLog(Log::LogLevel::Info, GetLocalizedString(StringResource::aborted).c_str()); return; }

		std::wstring childFolderObjId;
		PathT childDest{ currentDestPath };

		childDest /= srcfolder.filename();

		auto destFolder = destFolderMap.find(srcfolder.filename());
		if (destFolder == destFolderMap.end())
		{
			HRESULT hr = deviceContent.CreateFolder(currentFolderObjId, srcfolder.filename(), childFolderObjId);
			if (FAILED(hr))
			{
				WriteLog(Log::LogLevel::Error, GetLocalizedString(StringResource::failedToCreateFolder).c_str(), childDest.wstring().c_str());
				WriteLog(Log::LogLevel::Verbose, L"Error code: %x", hr);
				continue;
			}
		}
		else
		{
			childFolderObjId = destFolder->second;
		}
		if (!childFolderObjId.empty())
		{
			ReplicateFilesToPortableDevice(deviceContent, srcfolder, childFolderObjId, childDest);
		}
	}
}

StringT RepRunner::GetDurationString(const std::chrono::seconds& seconds)
{
	StringT str;
	int day = 0, hour = 0, min = 0, sec = 0;

	sec = static_cast<int>(seconds.count());
	min = sec / 60;
	sec %= 60;
	hour = min / 60;
	min %= 60;
	day = hour / 24;
	hour %= 24;

	CString resStr;

	if (day > 0)
	{
		if (resStr.LoadString((day == 1) ? IDS_TIMEFORMAT_DAY : IDS_TIMEFORMAT_DAYS))
			str += ToStringT(day) + (LPCTSTR)resStr;
		else
			str += ToStringT(day) + ((day == 1) ? _T(" day ") : _T(" days "));
	}
	if (hour > 0)
	{
		if (resStr.LoadString((hour == 1) ? IDS_TIMEFORMAT_HOUR : IDS_TIMEFORMAT_HOURS))
			str += ToStringT(hour) + (LPCTSTR)resStr;
		else
			str += ToStringT(hour) + ((hour == 1) ? _T(" hour ") : _T(" hours "));
	}
	if (min > 0)
	{
		if (resStr.LoadString((min == 1) ? IDS_TIMEFORMAT_MINUTE : IDS_TIMEFORMAT_MINUTES))
			str += ToStringT(min) + (LPCTSTR)resStr;
		else
			str += ToStringT(min) + ((min == 1) ? _T(" minute ") : _T(" minutes "));
	}
	if (resStr.LoadString((sec == 1) ? IDS_TIMEFORMAT_SECOND : IDS_TIMEFORMAT_SECONDS))
		str += ToStringT(sec) + (LPCTSTR)resStr;
	else
		str += ToStringT(sec) + ((sec == 1) ? _T(" second") : _T(" seconds"));

	return str;
}