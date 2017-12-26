#pragma once
#include <string>

class WinFile
{
public:
	WinFile(const std::wstring& file, DWORD accessModes, DWORD shareModes, DWORD creation, DWORD attributes);
	~WinFile();

	bool isGood() { return (m_file != INVALID_HANDLE_VALUE) ? true : false; }

	bool Read(void* buffer, DWORD toRead, DWORD& read)
	{
		return ReadFile(m_file, buffer, toRead, &read, nullptr);
	}
	bool Write(void* buffer, DWORD toWrite, DWORD& written)
	{
		return WriteFile(m_file, buffer, toWrite, &written, nullptr);
	}

	bool SetFileTime(const FILETIME* createdTime, const FILETIME* accessedTime, const FILETIME* modifiedTime)
	{
		return ::SetFileTime(m_file, createdTime, accessedTime, modifiedTime);
	}

	bool GetFileTime(FILETIME* createdTime, FILETIME* accessedTime, FILETIME* modifiedTime)
	{
		return ::GetFileTime(m_file, createdTime, accessedTime, modifiedTime);
	}

protected:
	HANDLE m_file;
};

