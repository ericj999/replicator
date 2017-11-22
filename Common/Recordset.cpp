#include "stdafx.h"
#include "Recordset.h"
#include "Table.h"

namespace Database
{

	Recordset::Recordset(const Table& tb, const StringT& condition, PropertyList& propList) :
		m_statement{ tb.m_db }, m_props( propList )
	{
		StringT fields;
		int index = 0;

		for (auto&& p : m_props)
		{
			if (index != 0)
				fields += _T(",");

			fields += p.m_name;

			p.m_index = index++;
			p.setNULL();
		}

		StringT sql = _T("SELECT ") + fields + _T(" FROM ") + tb.GetTableName();
		if (!condition.empty())
			sql += _T(" WHERE ") + condition;

		sql += _T(";");
		m_statement.Prepare(sql);
	}

	Recordset::~Recordset()
	{
	}

	// execute the prepared statement
	// return true - row returned
	//        fase - done
	// failed - exception
	bool Recordset::Step()
	{
		bool result = false;
		int ret = m_statement.Step();
		if (ret == SQLITE_ROW)
		{
			m_columnCount = m_statement.GetColumnCount();
			for (auto&& p : m_props)
			{
				p.setNULL();
				switch (p.m_type)
				{
				case PT_INT:
					if (m_statement.GetColumnType(p.m_index) == SQLITE_INTEGER)
					{
						p.m_i = m_statement.GetColumnValueInt(p.m_index);
						p.setNULL(false);
					}
					break;
				case PT_INT64:
					if (m_statement.GetColumnType(p.m_index) == SQLITE_INTEGER)
					{
						p.m_i64 = m_statement.GetColumnValueInt64(p.m_index);
						p.setNULL(false);
					}
					break;
				case PT_TEXT:
					if (m_statement.GetColumnType(p.m_index) == SQLITE3_TEXT)
					{
						p.m_str = m_statement.GetColumnValueText(p.m_index);
						p.setNULL(false);
					}
					break;
				}
			}
			result = true;
		}
		else if (ret != SQLITE_DONE)
			throw Exception(sqlite3_errstr(ret));

		return result;
	}
	int Recordset::GetColumnInt(int index, int defaultVal /*= 0*/)
	{
		if (index < GetColumnCount() && !m_props[index].IsNULL() && (m_props[index].m_type == PT_INT))
			return m_props[index].m_i;
		else
			return defaultVal;
	}
	int Recordset::GetColumnInt64(int index, int defaultVal /*= 0*/)
	{
		if (index < GetColumnCount() && !m_props[index].IsNULL() && (m_props[index].m_type == PT_INT64))
			return m_props[index].m_i;
		else
			return defaultVal;
	}
	StringT Recordset::GetColumnStr(int index, const StringT& defaultVal /*= _T("")*/)
	{
		if (index < GetColumnCount() && !m_props[index].IsNULL() && (m_props[index].m_type == PT_TEXT))
			return m_props[index].m_str;
		else
			return defaultVal;
	}
	StringT Recordset::GetColumnAsStr(int index, const StringT& defaultVal /*= _T("")*/)
	{
		StringT str(defaultVal);
		if (index < GetColumnCount() && !m_props[index].IsNULL())
		{
			if (m_props[index].m_type == PT_INT)
				str = ToStringT(m_props[index].m_i);
			else if (m_props[index].m_type == PT_INT64)
				str = ToStringT(m_props[index].m_i64);
			else if (m_props[index].m_type == PT_TEXT)
				str = m_props[index].m_str;
		}
		return str;
	}
}

