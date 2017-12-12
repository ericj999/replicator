#include "stdafx.h"
#include "FileTime.h"


FileTime::FileTime(std::wstring file) :
	m_valid{ false }, m_createdTime{ 0 }, m_accessedTime{ 0 }, m_modifiedTime{ 0 }
{
	HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (GetFileTime(hFile, &m_createdTime, &m_accessedTime, &m_modifiedTime))
			m_valid = true;

		CloseHandle(hFile);
	}
}


FileTime::~FileTime()
{
}
