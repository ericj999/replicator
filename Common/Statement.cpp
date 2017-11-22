#include "stdafx.h"
#include "Statement.h"

namespace Database
{
	Statement::Statement(Database& db) :
		m_db{ db }, m_stmt{ NULL }
	{
	}

	Statement::~Statement()
	{
		Finalize();
	}

	void Statement::Prepare(const StringT& sql)
	{
		Finalize();
		int ret = SQLitePrepare(m_db.GetDBHandle(), sql.c_str(), static_cast<int>((sql.length() + 1) * sizeof(TCHAR)), &m_stmt, NULL);
		if(ret != SQLITE_OK)
			throw Exception(sqlite3_errstr(ret));
	}

	int Statement::Step()
	{
		if (!m_stmt) throw Exception("There is no prepared statement!");
		int ret = sqlite3_step(m_stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW || ret == SQLITE_OK)
			return ret;
		else
			throw Exception(sqlite3_errstr(ret));
	}

	void Statement::Finalize()
	{
		if (m_stmt)
		{
			sqlite3_finalize(m_stmt);
			m_stmt = NULL;
		}
	}
}
