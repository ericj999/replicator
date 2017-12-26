#include "stdafx.h"
#include "WinFile.h"


WinFile::WinFile(const std::wstring& file, DWORD accessModes, DWORD shareModes, DWORD creation, DWORD attributes) :
	m_file{ INVALID_HANDLE_VALUE }
{
	m_file = CreateFile(file.c_str(), accessModes, shareModes, NULL, creation, attributes, NULL);
}


WinFile::~WinFile()
{
	if (m_file != INVALID_HANDLE_VALUE)
		CloseHandle(m_file);
}
