#pragma once
#include "StringT.h"

class Task
{
public:
	Task() {}
	~Task() {}

	void setID(__int64 id) { m_id = id; }
	void setName(std::basic_string<TCHAR>& name) { m_Name = name; }
	void setSource(std::basic_string<TCHAR>& source) { m_source = source; }
	void setDestination(std::basic_string<TCHAR>& dest) { m_dest = dest; }
	void setFlags(int flags) { m_flags = flags; }
	void setFilters(std::basic_string<TCHAR>& filters) { m_filters = filters; }
	void setCreatedTime(std::basic_string<TCHAR>& createdTime) { m_createdTime = createdTime; }
	void setLastRun(std::basic_string<TCHAR>& lastRun) { m_lastRun = lastRun; }

	__int64 getID() const { return m_id; }
	const std::basic_string<TCHAR>& getName() const { return m_Name; }
	const std::basic_string<TCHAR>& getSource() const { return m_source; }
	const std::basic_string<TCHAR>& getDestination() const { return m_dest; }
	int getFlags() { return m_flags; }
	const std::basic_string<TCHAR>& getFilters() { return m_filters; }
	const std::basic_string<TCHAR>& getCreatedTime() { return m_createdTime; }
	const std::basic_string<TCHAR>& getLastRun() { return m_lastRun; }

private:
	__int64 m_id;
	std::basic_string<TCHAR> m_Name;
	std::basic_string<TCHAR> m_source;
	std::basic_string<TCHAR> m_dest;
	int m_flags;
	std::basic_string<TCHAR> m_filters;
	std::basic_string<TCHAR> m_createdTime;
	std::basic_string<TCHAR> m_lastRun;
};

