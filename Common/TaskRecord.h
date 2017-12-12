#pragma once

#include "Database.h"
#include "Property.h"

class TaskRecord
{
public:
	TaskRecord(Database::Database& db, int taskID);
	~TaskRecord();

	struct ColumnDef
	{
		LPCTSTR colDBName;
		Database::PropertyType colDBType;
	};

	int getTaskID() { return m_taskID; }
	const StringT& getName() { return m_name;  }
	const StringT& getSource() { return m_source; }
	const StringT& getParsingSource() { return m_parsingSource; }
	const StringT& getDestination() { return m_destination;  }
	const StringT& getDestinationFolderFormat() { return m_destinationFolderFormat; }
	int getFlags() { return m_flags; }
	const StringT& getFilters() { return m_filters; }

protected:
	int m_taskID;
	StringT m_name;
	StringT m_source;
	StringT m_parsingSource;
	StringT m_destination;
	int m_flags;
	StringT m_filters;
	StringT m_destinationFolderFormat;
};

