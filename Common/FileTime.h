#pragma once

#include <string>
#include "ShellFolder.h"

class FileTime
{
public:
	FileTime(std::wstring file);
	FileTime(ShellWrapper::ShellItem2& shellItem);

	~FileTime();

	bool isValid() { return m_valid; }

	const FILETIME* getCreatedTime() const { return &m_createdTime; }
	const FILETIME* getAccessedTime() const { return &m_accessedTime; }
	const FILETIME* getModifiedTime() const { return &m_modifiedTime; }

protected:
	bool m_valid;
	FILETIME m_createdTime;
	FILETIME m_accessedTime;
	FILETIME m_modifiedTime;
};

