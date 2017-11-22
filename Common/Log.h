#pragma once

#include <mutex>
#include "StringT.h"

#define MUTEX_WAIT_TIME_SEC	1
#define MAX_LOG_FILE_SIZE	(4 * 1024 * 1024)
#define MAX_LOG_FILES		10

namespace Log
{
	enum LogLevel
	{
		Error,
		Warning,
		Info,
		Verbose
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
//		void Roll(bool force = false);
		void Roll();

	private:
		LogLevel m_logLevel;
		PathT m_path;
		int m_maxSize;
		int m_maxFiles;
		std::timed_mutex m_mutex;

		void CheckPath();
	};
}

