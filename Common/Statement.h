#pragma once

#include "Database.h"

namespace Database
{
	class Statement
	{
	public:
		Statement(Database& db);
		virtual ~Statement();

		void Prepare(const StringT& sql);
		int Step();
		void Finalize();

		int  GetColumnCount() { return sqlite3_column_count(m_stmt); }
		int GetColumnType(int index) { return sqlite3_column_type(m_stmt, index); }
		__int64 GetColumnValueInt64(int index) { return sqlite3_column_int64(m_stmt, index); }
		int GetColumnValueInt(int index) { return sqlite3_column_int(m_stmt, index); }
		StringT GetColumnValueText(int index)
		{
			LPCTSTR p = static_cast<LPCTSTR>(SQLiteColumnText(m_stmt, index));
			return StringT(p ? p : _T(""));
		}

		void Bind(int index, __int64 i64)
		{
			int ret = sqlite3_bind_int64(m_stmt, index, i64);
			if (ret != SQLITE_OK) throw Exception(sqlite3_errstr(ret));

		}
		void Bind(int index, int i)
		{
			int ret = sqlite3_bind_int(m_stmt, index, i);
			if (ret != SQLITE_OK) throw Exception(sqlite3_errstr(ret));
		}
		void Bind(int index, const StringT& str)
		{
			int ret = SQLiteBindText(m_stmt, index, str.c_str(), -1, NULL);
			if (ret != SQLITE_OK) throw Exception(sqlite3_errstr(ret));
		}

	private:
		Database& m_db;
		sqlite3_stmt* m_stmt;
	};
}

