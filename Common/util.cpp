#include "stdafx.h"
#include "util.h"
#include <Shlobj.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <Propsys.h>
#include <Propvarutil.h>
#include "GlobDef.h"
#include "FileTime.h"
#include "WinFile.h"
#include "ShellFolder.h"
#include "Log.h"

namespace Util
{
#define BUFSIZE (16 * 1024)

	PathT GetConfigPath()
	{
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);

		PathT path = szPath;
		if (!path.empty())
		{
			path /= STR_PRODUCT_NAME;

			if (!std::experimental::filesystem::exists(path))
				std::experimental::filesystem::create_directories(path);
		}
		else
		{
			GetModuleFileName(NULL, szPath, MAX_PATH);
			path = szPath;
			path.remove_filename();
		}
		return path;
	}

	PathT GetDatabasePath()
	{
		PathT dbPath{ GetConfigPath() };
		dbPath /= STR_DATABASE_FILE;
		return dbPath;
	}

	PathT GetProgramPath()
	{
		TCHAR szPath[MAX_PATH];
		PathT path;

		if (GetModuleFileName(NULL, szPath, _countof(szPath)) > 0)
		{
			path = szPath;
			path.remove_filename();
		}
		return path;
	}

	StringT GetDurationString(const std::chrono::seconds& seconds)
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

		if (day > 0)
			str += ToStringT(day) + ((day == 1) ? _T(" day ") : _T(" days "));

		if (hour > 0)
			str += ToStringT(hour) + ((hour == 1) ? _T(" hour ") : _T(" hours "));

		if (min > 0)
			str += ToStringT(min) + ((min == 1) ? _T(" minute ") : _T(" minutes "));

		str += ToStringT(sec) + ((sec == 1) ? _T(" second") : _T(" seconds"));

		return str;
	}

	StringT GetIsoTimeString(time_t tt)
	{
		struct tm stm;
		gmtime_s(&stm, &tt);

		return FormatTimeString(stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec);
	}

#define DATETIME_STRING_LEN 20

	StringT FormatTimeString(int year, int month, int day, int hour, int min, int sec)
	{
		StringT str(DATETIME_STRING_LEN, _T('\0'));

		_stprintf_s(const_cast<LPTSTR>(str.data()), DATETIME_STRING_LEN, _T("%.4u-%.02u-%.02u %.02u:%.02u:%.02u"), year, month, day, hour, min, sec);
		return str;
	}


	int CreateProcess(const PathT& program, const StringT& parameters, HANDLE* pHandle /*= NULL*/)
	{
		STARTUPINFO startupInfo;
		memset(&startupInfo, 0, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		startupInfo.wShowWindow = SW_HIDE;
		startupInfo.dwFlags = STARTF_USESHOWWINDOW;

		PROCESS_INFORMATION processInformation;

		if (pHandle)
			*pHandle = NULL;

		int iRet = 0;

		SECURITY_ATTRIBUTES saProc, saThread;

		memset(&saProc, 0, sizeof(saProc));
		saProc.nLength = sizeof(saProc);
		memset(&saThread, 0, sizeof(saThread));
		saThread.nLength = sizeof(saThread);

		StringT commandLine;
		PathT currentPath{ program };

		currentPath.remove_filename();
		commandLine = _T("\"") + program.wstring();
		if (parameters.empty())
			commandLine += _T("\"");
		else
			commandLine += _T("\" ") + parameters;

		BOOL result = ::CreateProcess(program.wstring().c_str(), const_cast<LPTSTR>(commandLine.c_str()), &saProc, &saThread, FALSE,
			CREATE_BREAKAWAY_FROM_JOB, NULL, const_cast<LPTSTR>(currentPath.wstring().c_str()), &startupInfo, &processInformation);

		if (result)
		{
			::CloseHandle(processInformation.hThread);
			if (pHandle)
				*pHandle = processInformation.hProcess;
			else
				::CloseHandle(processInformation.hProcess);
		}
		else
		{
			iRet = static_cast<int>(GetLastError());
		}
		return iRet;
	}

	bool CopyStreamToFile(ShellWrapper::ShellItem2& shellItem, const std::wstring& dest)
	{
		bool ret = false;
		HRESULT hr = E_FAIL;
		BYTE rgbFile[BUFSIZE];
		DWORD cbRead = 0;

		WinFile file{ dest.c_str(), GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL };

		if (file.isGood())
		{
			try
			{
				ShellWrapper::BindCtx bindCtx{ STGM_READ | STGM_SHARE_DENY_NONE };
				ShellWrapper::Stream stream;

				if (SUCCEEDED(hr = shellItem->BindToHandler(bindCtx.Get(), BHID_Stream, IID_IStream, (void**)&stream)))
				{
					ret = true;
					while (SUCCEEDED(hr = stream->Read(rgbFile, BUFSIZE, &cbRead)))
					{
						DWORD written = 0;
						if (cbRead == 0)
							break;

						if (!file.Write(rgbFile, cbRead, written) || (cbRead != written))
						{
							ret = false;
							break;
						}

						if (hr == S_FALSE)
							break;
					}
				}
			}
			catch (...)
			{
				ret = false;
			}
			if (ret)
			{
				FileTime itemTime{ shellItem };
				if (itemTime.isValid())
				{
					if (!file.SetFileTime(itemTime.getCreatedTime(), itemTime.getAccessedTime(), itemTime.getModifiedTime()))
					{
						DWORD err = GetLastError();
						Log::logger.error(StringT(L"Faile to set filetime. Code:") + ToStringT(err));
					}
				}
			}
		}
		else
		{
			Log::logger.error(StringT(L"Faile to open \"") + dest + StringT(L"\" for writing."));
		}
		return ret;
	}

	bool CopyFileToFolder(const PathT& src, ShellWrapper::ShellItem& shellFolder)
	{
		bool ret = false;
		HRESULT hr = E_FAIL;

		ShellWrapper::ShellItem srcItem;

		hr = SHCreateItemFromParsingName(src.wstring().c_str(), nullptr, IID_IShellItem, (void**)&srcItem);
		if (SUCCEEDED(hr))
		{
			ComInterface<IFileOperation> fileOperation{ CLSID_FileOperation };
			hr = fileOperation->SetOperationFlags(FOF_NO_UI | FOF_FILESONLY);
			if (FAILED(hr))
			{
				Log::logger.error(StringT(L"SetOperationFlags(FOF_NO_UI | FOF_FILESONLY) failed. Code:") + ToStringT(hr));
			}

			hr = fileOperation->CopyItem(srcItem.Get(), shellFolder.Get(), src.filename().c_str(), nullptr);
			if (SUCCEEDED(hr))
			{
				if (SUCCEEDED(hr = fileOperation->PerformOperations()))
				{
					ret = true;
				}
				else
				{
					Log::logger.error(StringT(L"Failed to perform copy operation for \"") + src.wstring() + StringT(L"\". Code:") + ToStringT(hr));
				}
			}
			else
			{
				Log::logger.error(StringT(L"Failed to set copy operation for \"") + src.wstring() + StringT(L"\". Code:") + ToStringT(hr));
			}
		}
		else
		{
			Log::logger.error(StringT(L"SHCreateItemFromParsingName() failed. Code:") + ToStringT(hr));
		}
		if (ret)
		{
			ShellWrapper::ShellItem newItem = shellFolder.OpenChildItem(src.filename().wstring().c_str());
			if (newItem.isValid())
			{
				ComInterface<IFileOperation> fileOperation{ CLSID_FileOperation };
				hr = fileOperation->SetOperationFlags(FOF_NO_UI);
				if (FAILED(hr))
				{
					Log::logger.error(StringT(L"SetOperationFlags(FOF_NO_UI) failed. Code:") + ToStringT(hr));
				}

				FileTime ft{ src.wstring() };
				if (ft.isValid())
				{
					PROPERTYKEY propKey[3] = { PKEY_DateCreated, PKEY_DateAccessed, PKEY_DateModified };
					PKA_FLAGS flag[3] = { PKA_SET, PKA_SET, PKA_SET };
					PROPVARIANT propValue[3];

					InitPropVariantFromFileTime(ft.getCreatedTime(), &propValue[0]);
					InitPropVariantFromFileTime(ft.getAccessedTime(), &propValue[1]);
					InitPropVariantFromFileTime(ft.getModifiedTime(), &propValue[2]);
					propValue[2].vt = VT_DATE;

					ComInterface<IPropertyChangeArray> changeArray;
					hr = PSCreatePropertyChangeArray(propKey, flag, propValue, 3, IID_IPropertyChangeArray, (void**)&changeArray);
					if (SUCCEEDED(hr))
					{
						hr = fileOperation->SetProperties(changeArray.Get());
						if (SUCCEEDED(hr))
						{
							hr = fileOperation->ApplyPropertiesToItem(newItem.Get());
							if(SUCCEEDED(hr))
								hr = fileOperation->PerformOperations();
							else
								Log::logger.error(StringT(L"IFileOperation::ApplyPropertiesToItem() failed. Code:") + ToStringT(hr));
						}
						else
							Log::logger.error(StringT(L"IFileOperation::SetProperties() failed. Code:") + ToStringT(hr));
					}
					else
						Log::logger.error(StringT(L"PSCreatePropertyChangeArray() failed. Code:") + ToStringT(hr));
				}
			}
			/*

			FileTime ft{ src.wstring() };
			if (ft.isValid())
			{
				// IStorage doesn't implement SetElementTimes
				ShellWrapper::ShellItem newItem = shellFolder.OpenChildItem(src.filename().wstring().c_str());
				if (newItem.isValid())
				{
					LPITEMIDLIST pidl = nullptr;
					hr = SHGetIDListFromObject(shellFolder.Get(), &pidl);
					if (SUCCEEDED(hr))
					{
						ComInterface<IPropertyStore> propStore;
						hr = SHGetPropertyStoreFromIDList(pidl, GPS_READWRITE, IID_IPropertyStore, (void**)&propStore);
						if (SUCCEEDED(hr))
						{
							PROPVARIANT propValue;
							InitPropVariantFromFileTime(ft.getModifiedTime(), &propValue);

							hr = propStore->SetValue(PKEY_DateCreated, propValue);
							if(SUCCEEDED(hr))
								hr = propStore->Commit();
						}
					}
				}
			}

			LPITEMIDLIST pidl = nullptr;
			hr = SHGetIDListFromObject(shellFolder.Get(), &pidl);
			if (SUCCEEDED(hr))
			{
				ShellWrapper::ShellFolder parent;
				ComInterface<IStorage> storage;
				PCUITEMID_CHILD pidlRelative = nullptr;

				hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&parent, &pidlRelative);
				if (SUCCEEDED(hr))
				{
					hr = parent->BindToStorage(pidlRelative, nullptr, IID_IStorage, (void**)&storage);
					if (SUCCEEDED(hr))
					{
						FileTime ft{ src.wstring() };
						if (ft.isValid())
						{
							hr = storage->SetElementTimes(src.filename().c_str(), ft.getCreatedTime(), ft.getAccessedTime(), ft.getModifiedTime());
						}
					}
				}
			}


			ShellWrapper::ShellItem newItem = shellFolder.OpenChildItem(src.filename().wstring().c_str());
			if (newItem.isValid())
			{
				ComInterface<IFileOperation> fileOperation{ CLSID_FileOperation };
				hr = fileOperation->SetOperationFlags(FOF_NO_UI | FOF_FILESONLY);
				if (FAILED(hr))
				{
					// failed to set NO_UI option
				}

				FileTime ft{ src.wstring() };
				if (ft.isValid())
				{
					PROPVARIANT propValue;

					PropVariantInit(&propValue);
					propValue.vt = VT_FILETIME;
					propValue.filetime = *ft.getCreatedTime();

					ComInterface<IPropertyChange> propChange;

					hr = PSCreateSimplePropertyChange(PKA_SET, PKEY_DateCreated, propValue, IID_IPropertyChange, (void**)&propChange);
					if (SUCCEEDED(hr))
					{
						PKA_FLAGS flag = PKA_SET;
						ComInterface<IPropertyChangeArray> changeArray;
						hr = PSCreatePropertyChangeArray(&PKEY_DateCreated, &flag, &propValue, 1, IID_IPropertyChangeArray, (void**)&changeArray);
						if (SUCCEEDED(hr))
						{
							hr = fileOperation->SetProperties(changeArray.Get());
							if (SUCCEEDED(hr))
								hr = fileOperation->PerformOperations();
						}
					}
				}
			}

			LPITEMIDLIST pidl = nullptr;

//			hr = SHGetIDListFromObject(shellFolder.Get(), &pidl);
//			if (SUCCEEDED(hr))
			{
				ComInterface<IStorage> storage;

//				hr = SHBindToParent(pidl, IID_IStorage, (void**)&storage, nullptr);
				hr = shellFolder->QueryInterface(IID_IStorage, (void**)&storage);
				if (SUCCEEDED(hr))
				{
					FileTime ft{ src.wstring() };
					if (ft.isValid())
					{
						hr = storage->SetElementTimes(src.filename().c_str(), ft.getCreatedTime(), ft.getAccessedTime(), ft.getModifiedTime());
					}
				}
			}

			ShellWrapper::ShellItem newItem = shellFolder.OpenChildItem(src.filename().wstring().c_str());
			if (newItem.isValid())
			{
				ShellWrapper::ShellItem2 newItem2;
				if (SUCCEEDED(hr = newItem->QueryInterface(IID_IShellItem2, (void**)&newItem2)))
				{
					PROPERTYKEY key = PKEY_DateModified;
					ComInterface<IPropertyStore> propStore;

					if (SUCCEEDED(hr = newItem2->GetPropertyStoreForKeys(&key, 1, GPS_READWRITE, IID_IPropertyStore, (void**)&propStore)))
					{
						FileTime ft{ src.wstring() };
						if (ft.isValid())
						{
							PROPVARIANT propValue;
							PropVariantInit(&propValue);
							propValue.vt = VT_FILETIME;
							propValue.filetime = *ft.getModifiedTime();

							hr = propStore->SetValue(PKEY_DateModified, propValue);
						}
					}
				}
				else
				{
					// cannot open the new item to set the times
				}
			}
			*/
		}
		return ret;
	}

	// doesn't work
	bool CopyFileToStream(const std::wstring& src, ShellWrapper::ShellItem& shellItem)
	{
		bool ret = false;
		HRESULT hr = E_FAIL;
		BYTE rgbFile[BUFSIZE];
		DWORD cbRead = 0;

		WinFile file{ src, GENERIC_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL };
		if (file.isGood())
		{
			try
			{
				ShellWrapper::BindCtx bindCtx{ STGM_WRITE | STGM_CREATE };
				ShellWrapper::Stream stream;

				if (SUCCEEDED(hr = shellItem->BindToHandler(bindCtx.Get(), BHID_Stream, IID_IStream, (void**)&stream)))
				{
					ret = true;

					while (file.Read(rgbFile, BUFSIZE, cbRead))
					{
						DWORD written = 0;
						if (cbRead == 0)
							break;
						
						if (FAILED(stream->Write(rgbFile, cbRead, &written)) || (cbRead != written))
						{
							ret = false;
							break;
						}

						if (hr == S_FALSE)
							break;
					}
				}
			}
			catch (...)
			{
				ret = false;
			}
			if (ret)
			{
				ShellWrapper::ShellItem2 item2;

				shellItem->QueryInterface(IID_IShellItem2, (void**)&item2);

				FileTime itemTime{ item2 };

				if (itemTime.isValid())
				{
					if (!file.SetFileTime(itemTime.getCreatedTime(), itemTime.getAccessedTime(), itemTime.getModifiedTime()))
					{
						DWORD err = GetLastError();
					}
				}
			}
		}
		return ret;
	}

	// Utility functions
	std::vector<std::wstring> ParseParsingPath(const std::wstring& path)
	{
		std::vector<std::wstring> list;

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type next = path.find_first_not_of(L"\\?", lastPos);
		std::wstring::size_type pos = path.find_first_of(L'\\', (next == std::wstring::npos) ? lastPos : next);

		for (;;)
		{
			if (pos == std::wstring::npos)
			{
				list.push_back(path.substr(lastPos));
				break;
			}
			else
				list.push_back(path.substr(lastPos, pos - lastPos));

			lastPos = pos + 1;
			next = path.find_first_not_of(L"\\?", lastPos);

			if (next == std::wstring::npos)
				break;	// end with \
													
			pos = path.find_first_of(L'\\', next);
		}
		return list;
	}
}