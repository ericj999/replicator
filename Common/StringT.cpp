#include "stdafx.h"
#include <time.h>
#include "StringT.h"
#include "DBDef.h"
#include "util.h"

namespace String
{
	StringT replace(const StringT& str, const StringT& toFind, const StringT& toReplace)
	{
		StringT s;

		size_t pos = 0;
		do
		{
			size_t startPos = pos;
			pos = str.find(toFind, startPos);
			if (pos != StringT::npos)
			{
				s += str.substr(startPos, pos - startPos) + toReplace;
				++pos;
			}
			else
				s += str.substr(startPos, pos);

		} while (pos != StringT::npos);

		return s;
	}

	StringT UTCTimeToLocalTime(const StringT& utcTime)
	{
		int year = 0, month = 0, day = 0, hr = 0, min = 0, sec = 0;

		if (_stscanf_s(utcTime.c_str(), _T("%d-%d-%d %d:%d:%d"), &year, &month, &day, &hr, &min, &sec) != 6)
			return StringT();

		SYSTEMTIME ut, lt;

		ut.wYear = year;
		ut.wMonth = month;
		ut.wDay = day;
		ut.wHour = hr;
		ut.wMinute = min;
		ut.wSecond = sec;
		ut.wMilliseconds = 0;

		FILETIME uft, lft;

		SystemTimeToFileTime(&ut, &uft);
		FileTimeToLocalFileTime(&uft, &lft);
		FileTimeToSystemTime(&lft, &lt);

		return Util::FormatTimeString(lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
	}

	void Tokenize(const StringT& str, std::vector<StringT>& tokens, const StringT& delimiters /*= _T(" ")*/)
	{
		// Skip delimiters at beginning.
		StringT::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		StringT::size_type pos = str.find_first_of(delimiters, lastPos);

		while ((pos != StringT::npos) || (lastPos != StringT::npos))
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

	StringT GetFiltersExp(const StringT& filters)
	{
		StringT str;
		std::vector<StringT> exts;
		
		Tokenize(filters, exts, STR_SRC_PATH_SEPARATOR);
		if (!exts.empty())
		{
			bool addBar = false;
			str = _T("\\.(");
			for (auto&& ext : exts)
			{
				if (addBar)
					str += _T("|");
				else
					addBar = true;

				str += ext;
			}
			str += _T(")$");
		}
		return str;
	}

#ifdef _UNICODE
	StringT StringToStringT(const std::string& str)
	{
		size_t size;
		mbstowcs_s(&size, nullptr, 0, str.c_str(), 0);
		wchar_t* wcs = new wchar_t[size + 1];
		mbstowcs_s(&size, wcs, size + 1, str.c_str(), size);
		StringT wstr{ wcs };
		delete[] wcs;
		return wstr;
	}

	std::string StringTToString(const StringT& str)
	{
		size_t size;
		wcstombs_s(&size, nullptr, 0, str.c_str(), 0);
		char* mbcs = new char[size + 1];
		wcstombs_s(&size, mbcs, size + 1, str.c_str(), size);
		std::string mbcsstr{ mbcs };
		delete[] mbcs;
		return mbcsstr;
	}
#endif
}
