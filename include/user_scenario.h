#pragma once
#include <sqlite3.h>

int user_access(sqlite3* db, int USER_ID);

int select_db_user(sqlite3* db, int USER_ID);