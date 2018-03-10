#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include "StringT.h"
#define __LOG_CPP__
#include "Log.h"

namespace Log
{
	Log logger;

	TCHAR LevelSymbol[static_cast<size_t>(LogLevel::max)] = { _T('E'), _T('W'), _T('I'), _T('V') };

	Log::Log() :
		m_logLevel{ Info }, m_maxSize{ MAX_LOG_FILE_SIZE }, m_maxFiles{ MAX_LOG_FILES }
	{

	}

	Log::Log(const PathT& logPath, int maxSize /*= MAX_LOG_FILE_SIZE*/, int maxFiles /*= MAX_LOG_FILES*/, LogLevel level /*=Info*/) :
		m_path{ logPath }, m_logLevel{ level }, m_maxSize{ maxSize }, m_maxFiles{ maxFiles }
	{
		CheckPath();
	}

	Log::~Log()
	{

	}

	void Log::Write(LogLevel level, const StringT& msg)
	{
		if (level <= m_logLevel && !m_path.empty())
		{
			if (m_mutex.try_lock_for(std::chrono::seconds(MUTEX_WAIT_TIME_SEC)))
			{
				bool roolLog = false;
				try
				{
					TCHAR buf[4096];
					std::basic_ofstream<TCHAR> fs{ m_path, std::ios_base::binary | std::ios_base::app };

					std::time_t t = std::time(nullptr);
					struct tm tmLocal;
					localtime_s(&tmLocal, &t);

					fs.rdbuf()->pubsetbuf(buf, _countof(buf));	// setting the wchar_t buffer so wchar_t can be written to file
					//	MS doesn't support %F, %T in put_time
					fs << std::put_time<TCHAR>(&tmLocal, _T("%Y-%m-%d %H:%M:%S <")) << getLevelSymbol(level) << _T(">[") << std::this_thread::get_id() << _T("] ") << msg << _T("\r\n");
					// roll the log if too big
					std::streampos pos = fs.tellp();
					if (pos > m_maxSize)
						roolLog = true;
				}
				catch (std::exception& e)
				{
					std::cerr << e.what();
				}
				if (roolLog) Roll();

				m_mutex.unlock();
			}
		}
	}

	void Log::setPath(const PathT& path)
	{
		m_path = path;
		CheckPath();
	}

	void Log::CheckPath()
	{
		if (!m_path.empty())
		{
			if (!std::experimental::filesystem::exists(m_path.parent_path()))
				std::experimental::filesystem::create_directories(m_path.parent_path());
		}
	}

	void Log::Roll()
	{
#if 0
		PathT oldLog{ m_path };
		oldLog.concat(_T(".old"));
		
		if (std::experimental::filesystem::exists(oldLog))
			std::experimental::filesystem::remove(oldLog);

		std::experimental::filesystem::rename(m_path, oldLog);
	}
#else
		try
		{
			if (!m_path.empty() && (m_maxFiles > 1) && std::experimental::filesystem::exists(m_path) && (std::experimental::filesystem::file_size(m_path) >= m_maxSize))
			{
				PathT log{ m_path.parent_path() };
				log /= m_path.stem().wstring() + ToStringT(m_maxFiles - 1) + m_path.extension().wstring();

				if (std::experimental::filesystem::exists(log))
					std::experimental::filesystem::remove(log);

				int i;
				for (i = m_maxFiles - 1; i > 0; --i)
				{
					PathT src{ m_path.parent_path() };
					PathT dest{ m_path.parent_path() };

					src /= m_path.stem().wstring() + ToStringT(i - 1) + m_path.extension().wstring();
					dest /= m_path.stem().wstring() + ToStringT(i) + m_path.extension().wstring();

					if (std::experimental::filesystem::exists(src))
						std::experimental::filesystem::rename(src, dest);
				}
				log = m_path.parent_path();
				log /= m_path.stem().wstring() + ToStringT(i) + m_path.extension().wstring();
				std::experimental::filesystem::rename(m_path, log);
			}
		}
		catch (...)
		{

		}
	}
#endif

	TCHAR Log::getLevelSymbol(LogLevel level)
	{
		if ((level >= LogLevel::Error) && (level < LogLevel::max))
			return LevelSymbol[static_cast<int>(level)];
		else
			return _T('\0');
	}
}
