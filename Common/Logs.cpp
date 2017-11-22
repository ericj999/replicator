#include "stdafx.h"
#include <iostream>
#include "Logs.h"

namespace Database
{
	Logs::Logs(Database& db) :
		m_db(db)
	{
	}

	void Logs::InsetLog(int task, LogLevel level, const StringT& msg)
	{
		StringT sql = _T("INSERT INTO LOGS (TaskID, Level, Message) VALUES (?1, ?2, ?3);");
		try
		{
			Statement stm{ m_db };
			stm.Prepare(sql);
			stm.Bind(1, task);
			stm.Bind(2, static_cast<int>(level));
			stm.Bind(3, msg);
			stm.Step();
		}
		catch (Exception& e)
		{
			std::cerr << e.what();
		}
	}
}