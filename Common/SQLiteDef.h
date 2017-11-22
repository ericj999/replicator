#pragma once

#include <SQLite\sqlite3.h>

#ifdef _UNICODE

#define SQLite3Open			sqlite3_open16
#define SQLiteErrMsg		sqlite3_errmsg16
#define SQLitePrepare		sqlite3_prepare16_v2
#define SQLiteBindText		sqlite3_bind_text16
#define SQLiteColumnText	sqlite3_column_text16

#else

#define SQLite3Open			sqlite3_open
#define SQLiteErrMsg		sqlite3_errmsg
#define SQLitePrepare		sqlite3_prepare_v2
#define SQLiteBindText		sqlite3_bind_text
#define SQLiteColumnText	sqlite3_column_text

#endif