#pragma once

// Tasks table
#define TASKS_TABLE					_T("Tasks")
#define TASKS_COL_TASKID			_T("TaskID")
#define TASKS_COL_NAME				_T("Name")
#define TASKS_COL_CREATEDTIME		_T("CreatedTime")
#define TASKS_COL_SOURCE			_T("Source")
#define TASKS_COL_SOURCE_PARSING	_T("SourceParsing")
#define TASKS_COL_DESTINATION		_T("Destination")
#define TASKS_COL_DEST_PARSING		_T("DestParsing")
#define TASKS_COL_FLASGS			_T("Flags")
#define TASKS_COL_FILTERS			_T("Filters")
#define TASKS_COL_LASTRUN			_T("LastRun")
#define TASKS_COL_LASTRUNSTATUS		_T("Status")
#define TASKS_COL_DESTFOLDERFMT		_T("DestFolderFormat")
#define TASKS_COL_LASTSUCCESSRUN	_T("LastSuccessfulRun")

#define HISTORY_TABLE				_T("History")
#define HISTORY_COL_TASKID			TASKS_COL_TASKID
#define HISTORY_COL_START_TIME		_T("StartTime")
#define HISTORY_COL_END_TIME		_T("EndTime")
#define HISTORY_COL_RESULT			_T("Result")

enum LogLevel
{
	Error,
	Warning,
	Info,
	Detail
};

#define STR_SRC_PATH_SEPARATOR	_T(";")

#define TASKS_FLAGS_INCLUDE_FILTERS			0x00000001
#define TASKS_FLAGS_EXCLUDE_FILTERS			0x00000002
#define TASKS_FLAGS_UPDATE_NEWER			0x00000010	// update destination if source is newer
#define TASKS_FLAGS_UPDATE_KEEP_BOTH		0x00000020	// copy the source to destination with appended index number
#define TASKS_FLAGS_UPDATE_DO_NOTHING		0x00000040	// do nothing if the destination and source are different
#define TASKS_FLAGS_UPDATE_OVERWRITE		0x00000080	// always overwrite the destination file
#define TASKS_FLAGS_UPDATE_SYNC				0x00000100	// remove the file from destination if the file doesn't exist in source
#define TASKS_FLAGS_DEST_GROUP_BY_DATE		0x00001000
#define TASKS_FLAGS_DEST_START_FROM_ROOT	0x00002000	// maintain same relative source path in destination folder
#define TASKS_FLAGS_ADV_OPT_MASKS			0x0000FFFF	// TASKS_FLAGS_INCLUDE_FILTERS, .., TASKS_FLAGS_UPDATE_SYNC

#define TASKS_FLAGS_INCLUDE_SUBDIR			0x00010000
#define TASKS_FLAGS_PORTABLE_DEVICE_SOURCE	0x10000000	// Windows portable device usb

