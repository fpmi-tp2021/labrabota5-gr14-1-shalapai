#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {

        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");

    return 0;
}

int select_db(sqlite3* db);
int modify_db(sqlite3* db);

int main()
{
    sqlite3* db;
    char* err_msg = 0;
    sqlite3_stmt* res = 0;

    int rc = sqlite3_open("auto_park.db", &db);

    if (rc != SQLITE_OK) {
        printf("Can't open database: %s.\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    else {
        printf("Opened database successfully.\n");
    }

    int isAllRight = 1;
    while (isAllRight)
    {
        printf("Choose the action:\n\t1) Show info\n\t2) Modify\n\t3) Exit\naction: ");
        int action_id;
        scanf("%d", &action_id);

        switch (action_id)
        {
        case 1:
            if (select_db(db))
                isAllRight = 0;
            break;
        case 2:
            if (modify_db(db))
                isAllRight = 0;
            break;
        case 3:
            printf("End of programm.\n");
            isAllRight = 0;
            break;
        default:
            printf("Wrong action_id.\n");
            isAllRight = 0;
            break;
        }
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return 0;
}

int select_db(sqlite3* db)
{


    return 0;
}

int modify_db(sqlite3* db)
{
    int col_number[3] = { 5, 7, 7 };

    char* col_names[3][7] = {
        {"id", "number", "brand", "initial_mileage", "capacity"},
        {"id", "number", "surname", "category", "experience", "address", "birth_year"},
        {"id", "date", "driver_id", "car_id", "mileage", "cargo_weight", "cost"}
    };

    char* insert[] = {
        "insert into vechicles(number, brand, initial_mileage, capacity) values(?, ?, ?, ?);",
        "insert into drivers(number, surname, category, experience, address, birth_year) values(?, ?, ?, ?, ?, ?);",
        "insert into orders(date, driver_id, car_id, mileage, cargo_weight, cost) values(?, ?, ?, ?, ?, ?);"
    };

    int table_id;
    
    printf("Enter table_id:\n\t1) vechicles\n\t2) drivers\n\t3) orders\ntable: ");
    scanf("%d", &table_id);
    if (table_id < 1 || table_id > 3) {
        printf("Wrong table_id.\n");
        return -1;
    }

    int action = 0;
    printf("Choose the action:\n\t1) Insert\n\t2) Update\n\t3) Delete\naction: ");
    scanf("%d", &action);
    if (action < 1 || action > 3) {
        printf("Invalid action id.\n");
        return -2;
    }

    switch (action)
    {
    case 1: {
        char values[6][100];
        printf("Enter values:\n");
        getchar();
        for (int i = 1; i < col_number[table_id - 1]; i++) {
            printf("\t%s: ", col_names[table_id - 1][i]);
            scanf("%99[^\n]", values[i - 1]);
            while (getchar() != '\n');
        }

        sqlite3_stmt* res;
        int rc = sqlite3_prepare_v2(db, insert[table_id - 1], -1, &res, 0);
        if (rc != SQLITE_OK) {
            printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return -4;
        }

        for (int i = 1; i < col_number[table_id - 1]; i++)
            sqlite3_bind_text(res, i, values[i - 1], -1, NULL);
        while (sqlite3_step(res) == SQLITE_ROW);
        break;
        }
    case 2:
        break;
    case 3:
        break;
    default:
        printf("Invalid action id.\n");
        return -3;
    }

    return 0;
}
