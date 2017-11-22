#include "stdafx.h"
#include "CRepCommandLineInfo.h"
#include "StringT.h"

const StringT verbose{ _T("verbose") };
const StringT testRun{ _T("testRun") };


CRepCommandLineInfo::CRepCommandLineInfo() :
	m_verbose{ false }, m_testRun{ false }
{
}


CRepCommandLineInfo::~CRepCommandLineInfo()
{
}

void CRepCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if (verbose.compare(pszParam) == 0)
		m_verbose = true;
	else if (testRun.compare(pszParam) == 0)
		m_testRun = true;
	else
		CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
}

