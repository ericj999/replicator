#include "stdafx.h"
#include "util.h"
#include <Shlobj.h>
#include "GlobDef.h"

namespace Util
{
	PathT GetConfigPath()
	{
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);

		PathT path = szPath;
		if (!path.empty())
		{
			path /= STR_PRODUCT_NAME;

			if (!std::tr2::sys::exists(path))
				std::tr2::sys::create_directories(path);
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
}