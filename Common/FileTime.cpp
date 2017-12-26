#include "stdafx.h"
#include "FileTime.h"
#include "WinFile.h"

FileTime::FileTime(std::wstring file) :
	m_valid{ false }, m_createdTime{ 0 }, m_accessedTime{ 0 }, m_modifiedTime{ 0 }
{
	WinFile fileObj{ file.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL };
	if (fileObj.isGood() && fileObj.GetFileTime(&m_createdTime, &m_accessedTime, &m_modifiedTime))
			m_valid = true;
}

FileTime::FileTime(ShellWrapper::ShellItem2& shellItem)
{
	if (SUCCEEDED(shellItem->GetFileTime(PKEY_DateModified, &m_modifiedTime)))
	{
		m_valid = true;
		if (FAILED(shellItem->GetFileTime(PKEY_DateAccessed, &m_accessedTime)))
			m_accessedTime = m_modifiedTime;

		if (FAILED(shellItem->GetFileTime(PKEY_DateCreated, &m_createdTime)))
			m_createdTime = m_modifiedTime;
	}
}

FileTime::~FileTime()
{
}
