#pragma once
#include "StringT.h"
#include <vector>
#include <functional>

#define STR_FF_DAY_2D			_T("DD")
#define STR_FF_DAY_ALT			_T("DDD")
#define STR_FF_WEEK_SHORT		_T("W")
#define STR_FF_WEEK_LONG		_T("WW")
#define STR_FF_WEEK_OF_YEAR		_T("U")
#define STR_FF_WEEK_OF_YEAR_ALT	_T("UU")
#define STR_FF_MONTH_2D			_T("MM")
#define STR_FF_MONTH_SHORT		_T("MMM")
#define STR_FF_MONTH_LONG		_T("MMMM")
#define STR_FF_YEAR_2D			_T("YY")
#define STR_FF_YEAR_4D			_T("YYYY")
#define STR_FF_YEAR_ALT			_T("YYYYY")

class DatePathFormatter
{
public:
	DatePathFormatter();
	DatePathFormatter(const StringT& format);

	~DatePathFormatter();

	void SetFormat(const StringT& format);

	StringT GetPath(std::tm* tms);

private:
	std::vector<std::function<StringT(std::tm* tms)>> m_funcs;
	StringT m_dayFormat;
	StringT m_weekFormat;
	StringT m_monthFormat;
	StringT m_yearFormat;

	const StringT GetGenericString(const StringT& str) { return str; }
	const StringT GetTimeFormatedString(const StringT& format, std::tm* tms);

	const StringT GetMacroFormatTemplate(const StringT& macro);
};

