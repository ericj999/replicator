#pragma once
#include <string>

namespace WPD
{
	void GetFileContentType(const std::wstring& ext, GUID& contentType, GUID& format);
}