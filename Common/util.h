#pragma once

#include "StringT.h"
#include <Windows.h>
#include <filesystem>
#include <chrono>
#include <time.h>

namespace Util
{
	PathT GetConfigPath();
	PathT GetDatabasePath();
	PathT GetProgramPath();

	StringT GetDurationString(const std::chrono::seconds& seconds);
	StringT GetIsoTimeString(time_t tt);
	StringT FormatTimeString(int year, int month, int day, int hour, int min, int sec);
	int CreateProcess(const PathT& program, const StringT& parameters, HANDLE* pHandle = NULL);
}