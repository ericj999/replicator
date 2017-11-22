#pragma once

#include "Table.h"
#include "Statement.h"
#include "DBDef.h"

namespace Database
{
	class Logs
	{
	public:
		Logs(Database& db);
		~Logs() {}

		void InsetLog(int task, LogLevel level, const StringT& msg);

	private:
		Database& m_db;
	};
}