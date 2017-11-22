#include "stdafx.h"
#include <iostream>
#include <assert.h>

#include "StringT.h"
#include "Database.h"

namespace Database
{
	Database::Database() :
		m_db{ NULL }
	{
	}


	Database::~Database()
	{
		Disconnect();
	}

	void Database::Connect(const StringT& db)
	{
		Disconnect();
		if (SQLite3Open(db.c_str(), &m_db) != SQLITE_OK)
		{
			if (!m_db)
				throw(std::bad_alloc());

			Exception e{ sqlite3_errmsg(m_db) };
			sqlite3_close(m_db);
			m_db = NULL;
			throw(e);
		}
	}

	void Database::Disconnect()
	{
		if (m_db)
		{
			sqlite3_close(m_db);
			m_db = NULL;
		}
	}

	void Database::Exec(const std::string& sql)
	{
		assert(m_db);
		char* errMsg = NULL;
		if (sqlite3_exec(m_db, sql.c_str(), NULL, NULL, &errMsg) != SQLITE_OK)
		{
			Exception e{ errMsg };
			sqlite3_free(errMsg);
			throw(e);
		}
	}
}