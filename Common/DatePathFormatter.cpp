#include "stdafx.h"
#include "DatePathFormatter.h"
#include <iomanip>
#include <functional>

const StringT STR_MACRO_LEFT{ _T("$(") };
const StringT STR_MACRO_RIGHT{ _T(")") };

DatePathFormatter::DatePathFormatter()
{
}

DatePathFormatter::DatePathFormatter(const StringT& format)
{
	SetFormat(format);
}

DatePathFormatter::~DatePathFormatter()
{
}

void DatePathFormatter::SetFormat(const StringT& format)
{
	m_funcs.clear();

	std::string::size_type posCurrent = 0;
	std::string::size_type pos = format.find(STR_MACRO_LEFT, posCurrent);

	while (pos != std::string::npos)
	{
		if (pos > posCurrent)
		{
			m_funcs.push_back(std::bind(&DatePathFormatter::GetGenericString, this, format.substr(posCurrent, pos - posCurrent)));
		}
		posCurrent = pos + STR_MACRO_LEFT.length();

		std::string::size_type posR = format.find(STR_MACRO_RIGHT, posCurrent);
		if (posR != std::string::npos)
		{
			m_funcs.push_back(std::bind(&DatePathFormatter::GetTimeFormatedString, this,
				GetMacroFormatTemplate(format.substr(posCurrent, posR - posCurrent)), std::placeholders::_1));

			posCurrent = posR + STR_MACRO_RIGHT.length();
			pos = format.find(STR_MACRO_LEFT, posCurrent);
		}
		else
		{
			posCurrent -= STR_MACRO_LEFT.length();
			break;
		}
	}

	if (posCurrent < format.length())
	{
		m_funcs.push_back(std::bind(&DatePathFormatter::GetGenericString, this, format.substr(posCurrent, format.length() - posCurrent)));
	}
}

StringT DatePathFormatter::GetPath(std::tm* tms)
{
	StringT str;

	for (auto elem : m_funcs)
		str.append(elem(tms));

	return str;
}

const StringT DatePathFormatter::GetTimeFormatedString(const StringT& format, std::tm* tms)
{
	StringStreamT ss;

	ss << std::put_time(tms, format.c_str());
	return ss.str();
}

const StringT DatePathFormatter::GetMacroFormatTemplate(const StringT& macro)
{
	StringT str = (macro.compare(STR_FF_DAY_2D) == 0) ? _T("%d") :
		(macro.compare(STR_FF_DAY_ALT) == 0) ? _T("%Ed") :
		(macro.compare(STR_FF_WEEK_SHORT) == 0) ? _T("%a") :
		(macro.compare(STR_FF_WEEK_LONG) == 0) ? _T("%A") : 
		(macro.compare(STR_FF_WEEK_OF_YEAR) == 0) ? _T("%U") :
		(macro.compare(STR_FF_WEEK_OF_YEAR_ALT) == 0) ? _T("%OU") :
		(macro.compare(STR_FF_MONTH_2D) == 0) ? _T("%m") :
		(macro.compare(STR_FF_MONTH_SHORT) == 0) ? _T("%b") : 
		(macro.compare(STR_FF_MONTH_LONG) == 0) ? _T("%B") : 
		(macro.compare(STR_FF_YEAR_2D) == 0) ? _T("%y") : 
		(macro.compare(STR_FF_YEAR_4D) == 0) ? _T("%Y") :
		(macro.compare(STR_FF_YEAR_ALT) == 0) ? _T("%EY") : _T("");

	return str;
}

