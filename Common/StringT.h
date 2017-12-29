#pragma once

#include <tchar.h>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <sstream>
#include <combaseapi.h>

using StringT  = std::basic_string<TCHAR>;

#ifdef _UNICODE

#define ToStringT std::to_wstring
#define StringToInt _wtol

using StringStreamT = std::wstringstream;
using RegexT = std::wregex;

#else

#define ToStringT std::to_string
#define StringToInt atol

using StringStreamT = std::stringstream;
using RegexT = std::regex;

#endif

using PathT = std::experimental::filesystem::path;
using DirectoryIteratorT = std::experimental::filesystem::directory_iterator;
using RecursiveDirectoryIteratorT = std::experimental::filesystem::recursive_directory_iterator;


namespace String
{
	StringT replace(const StringT& str, const StringT& toFind, const StringT& toReplace);
	StringT UTCTimeToLocalTime(const StringT& utcTime);

	template<typename T>
	void Tokenize(const std::basic_string<T>& str, std::vector<std::basic_string<T>>& tokens, const std::basic_string<T>& delimiters)
	{
		// Skip delimiters at beginning.
		std::basic_string<T>::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		std::basic_string<T>::size_type pos = str.find_first_of(delimiters, lastPos);

		while ((pos != std::basic_string<T>::npos) || (lastPos != std::basic_string<T>::npos))
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	};

	StringT GetFiltersExp(const StringT& filters);
#ifdef _UNICODE
	StringT StringToStringT(const std::string& str);
	std::string StringTToString(const StringT& str);
#else
	inline StringT StringToStringT(const std::string& str) { return str; }
	inline std::string StringTToString(const StringT& str) { return str; }
#endif
}

class GuidToString
{
public:
	GuidToString(const GUID& guid)
	{
		if (!::StringFromGUID2(guid, m_guidStr, sizeof(m_guidStr)))
		{
			m_guidStr[0] = L'\0';
		}
	}

	operator PCWSTR()
	{
		return m_guidStr;
	}
protected:
	WCHAR m_guidStr[128];
};