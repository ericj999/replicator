#pragma once

#include "StringT.h"
#include <Windows.h>
#include <filesystem>
#include <chrono>
#include <time.h>
#include "ShellFolder.h"

namespace Util
{
	PathT GetConfigPath();
	PathT GetDatabasePath();
	PathT GetProgramPath();

	StringT GetIsoTimeString(time_t tt);
	StringT FormatTimeString(int year, int month, int day, int hour, int min, int sec);

	std::vector<std::wstring> ParseParsingPath(const std::wstring& path);

	int CreateProcess(const PathT& program, const StringT& parameters, HANDLE* pHandle = NULL);
	bool CopyStreamToFile(ShellWrapper::ShellItem2& shellItem, const std::wstring& dest);
	bool CopyFileToFolder(const PathT& src, ShellWrapper::ShellItem& shellFolder);
	bool CopyFileToStream(const std::wstring& src, ShellWrapper::ShellItem& shellItem);
}