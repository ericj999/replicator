#pragma once

#include <vector>
#include <memory>
#include "Database.h"
#include "Statement.h"
#include "StringT.h"
#include "Property.h"
#include "Recordset.h"

namespace Database
{
	class Table
	{
	public:
		Table(Database& db, const StringT& table);
		~Table();

		// attributes
		const StringT& GetTableName() const { return m_table; }

		// methods
		RecordsetPtr Select(PropertyList& propList, const StringT& condition = _T(""));
		void Insert(PropertyList& propList);
		void Update(PropertyList& propList, const StringT& condition);
		void Delete(const StringT& condition);
		int GetCount(Property& prop);	// property contains col name cand value

	private:
		Database& m_db;
		const StringT m_table;

		void BindStatementPRoperties(Statement& stm, PropertyList& propList);

		friend class Recordset;
	};
}

