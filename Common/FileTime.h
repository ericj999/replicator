#pragma once

#include <string>

class FileTime
{
public:
	FileTime(std::wstring file);
	~FileTime();

	bool isValid() { return m_valid; }

	FILETIME& getCreatedTime() { return m_createdTime; }
	FILETIME& getAccessedTime() { return m_accessedTime; }
	FILETIME& getModifiedTime() { return m_modifiedTime; }

protected:
	bool m_valid;
	FILETIME m_createdTime;
	FILETIME m_accessedTime;
	FILETIME m_modifiedTime;
};

