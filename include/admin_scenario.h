#pragma once
#include <sqlite3.h>

int select_db(sqlite3* db);
int modify_db(sqlite3* db);

int choose_table();
int check_cargo_weight(sqlite3* db, int id, int cargo_weight);
int drivers_salary_for_the_period(sqlite3* db, char* start_date, char* end_date);
int driver_salary_for_the_period(sqlite3* db, char* start_date, char* end_date, char* surname);

int admin_access(sqlite3* db);

int callback(void* NotUsed, int argc, char** argv, char** azColName);