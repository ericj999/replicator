#pragma once

#include <mutex>
#include "StringT.h"

#define MUTEX_WAIT_TIME_SEC	1
#define MAX_LOG_FILE_SIZE	(10 * 1024 * 1024)
#define MAX_LOG_FILES		10

namespace Log
{
	enum LogLevel
	{
		Error = 0,
		Warning,
		Info,
		Verbose,
		max
	};

	class Log
	{
	public:
		Log();
		Log(const PathT& logPath, int maxSize = MAX_LOG_FILE_SIZE, int maxFiles = MAX_LOG_FILES, LogLevel level = Verbose);
		~Log();

		// properties
		void setPath(const PathT& path);
		void setMaxSize(int size) { m_maxSize = size; }
		void setMaxFiles(int count) { m_maxFiles = count; }
		void setLevel(LogLevel level) { m_logLevel = level; }

		// methods
		void Write(LogLevel level, const StringT& msg);

		void info(const StringT& msg) { Write(LogLevel::Info, msg); }
		void error(const StringT& msg) { Write(LogLevel::Error, msg); }
		void warn(const StringT& msg) { Write(LogLevel::Warning, msg); }
		void verbose(const StringT& msg) { Write(LogLevel::Verbose, msg); }

	protected:
		LogLevel m_logLevel;
		PathT m_path;
		int m_maxSize;
		int m_maxFiles;
		std::timed_mutex m_mutex;

		void CheckPath();
		void Roll();
		TCHAR getLevelSymbol(LogLevel level);
	};

#ifndef __LOG_CPP__
	extern Log logger;
#endif
}
