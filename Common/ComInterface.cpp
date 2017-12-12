#include "stdafx.h"
#include "ComInterface.h"

std::string StringTToString(const std::wstring& wstr)
{
	size_t size;
	wcstombs_s(&size, nullptr, 0, wstr.c_str(), 0);
	char* mbcs = new char[size + 1];
	wcstombs_s(&size, mbcs, size + 1, wstr.c_str(), size);
	std::string str{ mbcs };
	delete[] mbcs;
	return str;
}

std::string IIDToString(REFIID iid)
{
	std::string iidStr;
	LPOLESTR psz = nullptr;

	if (SUCCEEDED(StringFromIID(iid, &psz)) && psz)
	{
		iidStr = StringTToString(psz);
		CoTaskMemFree(psz);
	}
	return iidStr;
}