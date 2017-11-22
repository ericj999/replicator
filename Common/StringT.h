#pragma once

#include <tchar.h>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <sstream>

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
	void Tokenize(const StringT& str, std::vector<StringT>& tokens, const StringT& delimiters = _T(" "));
	StringT GetFiltersExp(const StringT& filters);
#ifdef _UNICODE
	StringT StringToStringT(const std::string& str);
#else
	inline StringT StringToStringT(const std::string& str) { return str; }
#endif
}