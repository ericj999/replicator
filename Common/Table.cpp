#include "stdafx.h"
#include "Table.h"
#include "statement.h"
#include "Recordset.h"

namespace Database
{
	Table::Table(Database& db, const StringT& table) :
		m_db{ db }, m_table( table )
	{
	}

	Table::~Table()
	{
	}

	std::shared_ptr<Recordset> Table::Select(PropertyList& propList, const StringT& condition /*= _T("")*/)
	{
		return std::make_shared<Recordset>(*this, condition, propList);
	}

	void Table::Insert(PropertyList& propList)
	{
		if (propList.empty()) throw Exception("No value specified.");

		StringT names;
		StringT values;
		int index = 0;

		for (auto&& p : propList)
		{
			if (index != 0)
			{
				names += _T(',');
				values += _T(',');
			}
			p.m_index = index;
			names += p.m_name;
			values += _T("?") + ToStringT(index + 1);
			++index;
		}

		StringT sql = _T("INSERT INTO ") + m_table + _T(" (") + names + _T(") VALUES (") + values + _T(");");

		Statement stm{ m_db };
		stm.Prepare(sql);
		BindStatementPRoperties(stm, propList);
		stm.Step();
	}

	void Table::Update(PropertyList& propList, const StringT& condition)
	{
		if (propList.empty()) throw Exception("No value specified.");

		StringT setList;

		int index = 0;

		for (auto&& p : propList)
		{
			if (index != 0)
				setList += _T(',');

			p.m_index = index;
			setList += p.m_name + _T("=?") + ToStringT(index + 1);
			++index;
		}

		StringT sql = _T("UPDATE ") + m_table + _T(" SET ") + setList + _T(" WHERE ") + condition + _T(";");

		Statement stm{ m_db };
		stm.Prepare(sql);
		BindStatementPRoperties(stm, propList);
		stm.Step();
	}

	void Table::Delete(const StringT& condition)
	{
		StringT sql = _T("DELETE FROM ") + m_table;
		if (!condition.empty())
			sql += _T(" WHERE ") + condition;

		Statement stm{ m_db };
		stm.Prepare(sql);
		stm.Step();
	}

	int Table::GetCount(Property& prop)
	{
		int count = 0;
		StringT value;
		if (prop.m_type == PT_TEXT)
			value = _T("'") + String::replace(prop.ToString(), _T("'"), _T("''")) + _T("'");
		else
			value = prop.ToString();

		StringT condition = _T("(") + prop.m_name + _T(" = ") + value + _T(");");
		StringT sql = _T("SELECT COUNT(*) FROM ") + m_table + _T(" WHERE ") + condition;

		Statement stm{ m_db };
		stm.Prepare(sql);
		if (stm.Step() && (stm.GetColumnCount() == 1) && (stm.GetColumnType(0) == SQLITE_INTEGER))
			count = stm.GetColumnValueInt(0);

		return count;
	}


	void Table::BindStatementPRoperties(Statement& stm, PropertyList& propList)
	{
		for (auto&& p : propList)
		{
			if (p.m_index >= 0)
			{
				if (p.m_type == PT_INT64)
					stm.Bind(p.m_index + 1, p.m_i64);
				else if (p.m_type == PT_INT)
					stm.Bind(p.m_index + 1, p.m_i);
				else
					stm.Bind(p.m_index + 1, p.m_str);
			}
		}
	}
}