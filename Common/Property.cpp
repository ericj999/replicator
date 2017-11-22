#include "stdafx.h"
#include "Property.h"

namespace Database
{
	Property::Property() :
		m_type{ PT_NULL }, m_i64{ 0 }, m_isNULL{ true }
	{}
	Property::Property(const StringT& name, __int64 i64, int index) :
		m_name(name), m_type{ PT_INT64 }, m_i64{ i64 }, m_index{ index }, m_isNULL{ false }
	{}
	Property::Property(const StringT& name, int i, int index) :
		m_name(name), m_type{ PT_INT }, m_i{ i }, m_index{ index }, m_isNULL{ false }
	{}
	Property::Property(const StringT& name, const StringT& str, int index) :
		m_name(name), m_type{ PT_TEXT }, m_i64{ 0 }, m_str(str), m_index{ index }, m_isNULL{ false }
	{}
	Property::Property(const StringT& name, LPCTSTR str, int index) :
		m_name(name), m_type{ PT_TEXT }, m_i64{ 0 }, m_str(str), m_index{ index }, m_isNULL{ false }
	{}
	Property::Property(const StringT& name, PropertyType pt) :
		m_name(name), m_type{ pt }, m_i64{ 0 }, m_isNULL{ true }
	{}
	Property::Property(const Property& p) :
		m_name(p.m_name), m_type{ p.m_type }, m_str(p.m_str), m_i64{ p.m_i64 }, m_index{ p.m_index }, m_isNULL{ p.m_isNULL }
	{}

	const StringT& Property::ToString()
	{
		if (m_type == PT_INT)
			m_str = ToStringT(m_i);
		else if (m_type == PT_INT64)
			m_str = ToStringT(m_i64);

		return m_str;
	}
}