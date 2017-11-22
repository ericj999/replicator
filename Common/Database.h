#pragma once

#include "StringT.h"
#include "SQLiteDef.h"

namespace Database
{
	class Exception : public std::runtime_error
	{
	public:
		Exception(const char* err) : runtime_error(err) {}
		~Exception() {}
	};

	class Database
	{
	public:
		Database();
		~Database();

		void Connect(const StringT& db);
		void Disconnect();
		void Exec(const std::string& sql);

		sqlite3* GetDBHandle() { return m_db; }

	private:
		sqlite3* m_db;
	};
}
