#pragma once

// CCommandLineInfo
class CRepCommandLineInfo : public CCommandLineInfo
{
public:
	CRepCommandLineInfo();
	~CRepCommandLineInfo();

	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

	bool m_verbose;
	bool m_testRun;
};
