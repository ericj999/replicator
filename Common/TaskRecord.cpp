#include "stdafx.h"
#include "DBDef.h"

#include "TaskRecord.h"
#include "Table.h"
#include "Recordset.h"

TaskRecord::ColumnDef columns[] =
{
	{ TASKS_COL_NAME, Database::PT_TEXT },
	{ TASKS_COL_SOURCE, Database::PT_TEXT },
	{ TASKS_COL_SOURCE_PARSING, Database::PT_TEXT },
	{ TASKS_COL_DESTINATION, Database::PT_TEXT },
	{ TASKS_COL_FLASGS, Database::PT_INT },
	{ TASKS_COL_FILTERS, Database::PT_TEXT },
	{ TASKS_COL_DESTFOLDERFMT, Database::PT_TEXT }
};

TaskRecord::TaskRecord(Database::Database& db, int taskID) :
	m_flags{ 0 }
{
	Database::Table tb{ db, TASKS_TABLE };
	StringT condition = _T("TaskID=") + ToStringT(taskID);

	Database::PropertyList props;
	for (int i = 0; i < (sizeof(columns) / sizeof(columns[0])); ++i)
		props.push_back(Database::Property(columns[i].colDBName, columns[i].colDBType));

	Database::RecordsetPtr rs = tb.Select(props, condition);
	if (rs->Step())
	{
		m_taskID = taskID;

		const Database::Property& taskName = props.Find(TASKS_COL_NAME);
		if (!taskName.IsNULL()) m_name = taskName.m_str;

		const Database::Property& src = props.Find(TASKS_COL_SOURCE);
		if (!src.IsNULL()) m_source = src.m_str;

		const Database::Property& parsingSrc = props.Find(TASKS_COL_SOURCE_PARSING);
		if (!parsingSrc.IsNULL()) m_parsingSource = parsingSrc.m_str;

		const Database::Property& dest = props.Find(TASKS_COL_DESTINATION);
		if (!dest.IsNULL()) m_destination = dest.m_str;

		const Database::Property& destFolderFormat = props.Find(TASKS_COL_DESTFOLDERFMT);
		if (!destFolderFormat.IsNULL()) m_destinationFolderFormat = destFolderFormat.m_str;

		const Database::Property& flags = props.Find(TASKS_COL_FLASGS);
		if (!flags.IsNULL()) m_flags = flags.m_i;

		const Database::Property& filters = props.Find(TASKS_COL_FILTERS);
		if (!src.IsNULL()) m_filters = filters.m_str;
	}
}

TaskRecord::~TaskRecord()
{
}
