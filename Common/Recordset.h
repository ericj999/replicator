#pragma once
#include <memory>
#include "Database.h"
#include "Statement.h"
#include "Property.h"
#include "StringT.h"

namespace Database
{
	class Table;

	class Recordset
	{
	public:
		Recordset(const Table& tb, const StringT& condition, PropertyList& propList);
		~Recordset();

		bool Step();

		int GetColumnCount() { return m_columnCount; }
		int GetColumnInt(int index, int defaultVal = 0);
		int GetColumnInt64(int index, int defaultVal = 0);
		StringT GetColumnStr(int index, const StringT& defaultVal = _T(""));
		StringT GetColumnAsStr(int index, const StringT& defaultVal = _T(""));

	protected:
		PropertyList& m_props;
		Statement m_statement;
		int m_columnCount;
	};

	using RecordsetPtr = std::shared_ptr<Recordset>;
}