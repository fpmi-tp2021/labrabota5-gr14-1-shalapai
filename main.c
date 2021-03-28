#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    printf("\n\n***********************************************\n");
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("***********************************************\n\n");
    return 0;
}

int select_db(sqlite3* db);
int modify_db(sqlite3* db);

int choose_table();
int check_cargo_weight(sqlite3* db, int id, int cargo_weight);
int drivers_salary_for_the_period(sqlite3* db, char* start_date, char* end_date);
int driver_salary_for_the_period(sqlite3* db, char* start_date, char* end_date, char* surname);

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

int driver_salary_for_the_period(sqlite3* db, char* start_date, char* end_date, char* surname)
{
    char* query = "select drivers.id, drivers.surname, sum(orders.cost) * 0.2 as salary,\
                   min(orders.date) as start_date,\
                   max(orders.date) as end_date from drivers inner join orders on drivers.id=orders.driver_id\
                   where orders.date between ? and ? and drivers.surname = ? group by drivers.id=orders.driver_id;";

    sqlite3_stmt* res;
    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(res, 1, start_date, -1, NULL);
    sqlite3_bind_text(res, 2, end_date, -1, NULL);
    sqlite3_bind_text(res, 3, surname, -1, NULL);

    while (sqlite3_step(res) == SQLITE_ROW) {
        printf("\n***********************************************\n");
        for (int i = 0; i < 5; i++)
            printf("%s - %s\n", sqlite3_column_name(res, i), sqlite3_column_text(res, i));
        printf("\n***********************************************\n\n");
    }
    sqlite3_finalize(res);

    return 1;
}

int drivers_salary_for_the_period(sqlite3* db, char* start_date, char* end_date)
{
    char* create_table = "drop table if exists salary_for_period;\
                          create table salary_for_period(id, surname, salary, start_date, end_date);";

    char* query = "insert into salary_for_period select drivers.id, drivers.surname, sum(orders.cost) * 0.2 as salary,\
                   min(orders.date) as start_date,\
                   max(orders.date) as end_date from drivers inner join orders on drivers.id=orders.driver_id\
                   where orders.date between ? and ? group by orders.driver_id;";

    char* err_msg;
    int rc = sqlite3_exec(db, create_table, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }

    sqlite3_stmt* res;
    rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(res, 1, start_date, -1, NULL);
    sqlite3_bind_text(res, 2, end_date, -1, NULL);

    while (sqlite3_step(res) == SQLITE_ROW);
    sqlite3_finalize(res);

    return 1;
}

int check_cargo_weight(sqlite3* db, int id, int cargo_weight)
{
    char* query = "select capacity from vechicles where id = ?";
    sqlite3_stmt* res;
    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(res, 1, id);

    if (!(sqlite3_step(res) == SQLITE_ROW))
    {
        printf("Cannot perform query.\n");
        sqlite3_finalize(res);
        return 0;
    }
    int capacity = sqlite3_column_int(res, 0);
    if (capacity >= cargo_weight) {
        sqlite3_finalize(res);
        return 1;
    }

    sqlite3_finalize(res);
    return 0;
}

int choose_table()
{
    int table_id = 0;

    char* tables[] = {
        "users",
        "vechicles",
        "drivers",
        "orders"
    };

    int tables_size = sizeof(tables) / sizeof(tables[0]);
    printf("Enter table_id:");
    for (int i = 0; i < tables_size; i++) {
        printf("\n\t%d) %s", i + 1, tables[i]);
    }
    printf("\ntable: ");
    scanf("%d", &table_id);
    if (table_id < 1 || table_id > tables_size) {
        printf("Wrong table_id.\n");
        table_id = 0;
    }

    return table_id;
}

int select_db(sqlite3* db)
{
    char* query_name[] = {
        "show all information about a specified table",
        "for the specified driver - a list of completed orders for the period",
        "for the specified vehicle - total mileage and total weight of transported goods",
        "for each driver - total number of trips, total weight transported goods, the amount of money earned",
        "for the driver who completed the least number of trips - all information and the amount of money received",
        "for a car with the highest total mileage - all information",
        "determines the amount of money for the specified period, credited to each driver for transportation.",
        "determines the amount of money for the specified period, credited to the specified driver for transportation."
    };

    char* select_all[] = {
        "select * from users;",
        "select * from vechicles;",
        "select * from drivers;",
        "select * from orders;"
    };

    char* queries[] = {
        "select * from orders where driver_id=? and date between ? and ?",
        "select vechicles.id, vechicles.number, vechicles.brand,\
         sum(orders.mileage) + vechicles.initial_mileage as total_mileage, sum(orders.cargo_weight)\
         as total_weight from vechicles inner join orders on vechicles.id=orders.car_id where vechicles.id=?;",
        "select drivers.id, drivers.surname, count(*) as total_orders, sum(orders.cargo_weight) as total_weight,\
         sum(orders.cost) * 0.2 as earned from drivers inner join orders on drivers.id=orders.driver_id\
         group by orders.driver_id;",
        "select id, number, surname, category, experience, address, birth_year, min(total) as total_orders,\
         cost * 0.2 as earned from(select count(*) as total, d.id, d.number, d.surname,\
         d.category, d.experience, d.address, d.birth_year,\
         sum(orders.cost) as cost from drivers as d inner join orders on d.id = orders.driver_id group by orders.driver_id);",
        "select id, number, brand, initial_mileage, capacity, max(total) as total_mileage\
         from(select v.id, v.number, v.brand, v.initial_mileage, v.capacity, sum(orders.mileage) + v.initial_mileage as total\
         from vechicles as v inner join orders on v.id = orders.car_id group by orders.car_id);"
    };

    int col_number[2] = {7, 5};

    int count_queries = sizeof(query_name) / sizeof(query_name[0]);
    int query_id = 0;
    printf("Choose the query:");

    for (int i = 0; i < count_queries; i++) {
        printf("\n\t%d) %s", i + 1, query_name[i]);
    }
    printf("\nquery: ");

    scanf("%d", &query_id);
    if (query_id < 1 || query_id > count_queries) {
        printf("Invalid query id.\n");
        return -2;
    }

    if (query_id == 1) {
        int table_id = choose_table();
        if (!table_id)
            return -1;

        char* err_msg = 0;

        int rc = sqlite3_exec(db, select_all[table_id - 1], callback, 0, &err_msg);

        if (rc != SQLITE_OK) {

            printf("Failed to select data\n");
            printf("SQL error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return -3;
        }
    }
    else if (query_id == 7) {
        char date1[11];
        char date2[11];

        printf("Enter dates in format yyyy-mm-dd:\n\tstart date: ");
        scanf("%10s", date1);
        printf("\tend date: ");
        scanf("%10s", date2);

        if (!drivers_salary_for_the_period(db, date1, date2)) {
            printf("Cannot perform query.\n");
            return -7;
        }

        char* err_msg;
        int rc = sqlite3_exec(db, "select * from salary_for_period", callback, 0, &err_msg);

        if (rc != SQLITE_OK) {
            printf("SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            return -8;
        }
    }
    else if (query_id == 8) {
        char date1[11];
        char date2[11];
        char surname[30];

        printf("Enter dates in format yyyy-mm-dd:\n\tstart date: ");
        scanf("%10s", date1);
        printf("\tend date: ");
        scanf("%10s", date2);

        printf("Enter driver surename: ");
        scanf("%29s", surname);

        if (!driver_salary_for_the_period(db, date1, date2, surname)) {
            printf("Cannot perform query.\n");
            return -8;
        }
    }
    else
    {
        query_id -= 2;
        if (query_id == 0) {

            sqlite3_stmt* res;
            int rc = sqlite3_prepare_v2(db, queries[query_id], -1, &res, 0);
            if (rc != SQLITE_OK) {
                printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
                return -4;
            }

            int id;
            char date1[11];
            char date2[11];

            printf("Enter id: ");
            scanf("%d", &id);

            printf("Enter dates in format yyyy-mm-dd:\n\tstart date: ");
            scanf("%10s", date1);
            printf("\tend date: ");
            scanf("%10s", date2);

            sqlite3_bind_int(res, 1, id);
            sqlite3_bind_text(res, 2, date1, -1, NULL);
            sqlite3_bind_text(res, 3, date2, -1, NULL);

            while (sqlite3_step(res) == SQLITE_ROW) {
                printf("\n***********************************************\n");
                for (int i = 0; i < col_number[query_id]; i++)
                    printf("%s - %s\n", sqlite3_column_name(res, i), sqlite3_column_text(res, i));
                printf("\n***********************************************\n\n");
            }
            sqlite3_finalize(res);
        }
        else if (query_id == 1) {
            sqlite3_stmt* res;
            int rc = sqlite3_prepare_v2(db, queries[query_id], -1, &res, 0);
            if (rc != SQLITE_OK) {
                printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
                return -5;
            }

            int id;

            printf("Enter id: ");
            scanf("%d", &id);

            sqlite3_bind_int(res, 1, id);

            while (sqlite3_step(res) == SQLITE_ROW) {
                printf("\n***********************************************\n");
                for (int i = 0; i < col_number[query_id]; i++)
                    printf("%s - %s\n", sqlite3_column_name(res, i), sqlite3_column_text(res, i));
                printf("\n***********************************************\n\n");
            }
            sqlite3_finalize(res);
        }
        else {
            char* err_msg;
            int rc = sqlite3_exec(db, queries[query_id], callback, 0, &err_msg);

            if (rc != SQLITE_OK) {
                printf("SQL error: %s\n", err_msg);

                sqlite3_free(err_msg);

                return -6;
            }
        }
    }

    return 0;
}

int modify_db(sqlite3* db)
{
    sqlite3_stmt* res = 0;

    int col_number[4] = { 3, 5, 7, 7 };

    char* col_names[4][7] = {
        {"id", "username", "password"},
        {"id", "number", "brand", "initial_mileage", "capacity"},
        {"id", "number", "surname", "category", "experience", "address", "birth_year"},
        {"id", "date", "driver_id", "car_id", "mileage", "cargo_weight", "cost"}
    };

    char* insert[] = {
        "insert into users(username, password) values(?, ?);",
        "insert into vechicles(number, brand, initial_mileage, capacity) values(?, ?, ?, ?);",
        "insert into drivers(number, surname, category, experience, address, birth_year) values(?, ?, ?, ?, ?, ?);",
        "insert into orders(date, driver_id, car_id, mileage, cargo_weight, cost) values(?, ?, ?, ?, ?, ?);"
    };

    char* delete[] = {
        "delete from users where id = ?;",
        "delete from vechicles where id = ?;",
        "delete from drivers where id = ?;",
        "delete from orders where id = ?;"
    };

    char* update[] = {
        "update users set username = ?, password = ? where id = ?;",
        "update vechicles set number = ?, brand = ?, initial_mileage = ?, capacity = ? where id = ?;",
        "update drivers set number = ?, surname = ?, category = ?, experience = ?, address = ?, birth_year = ? where id = ?;",
        "update orders set date = ?, driver_id = ?, car_id = ?, mileage = ?, cargo_weight = ?, cost = ? where id = ?;"
    };

    int table_id = choose_table();
    if (!table_id)
        return -1;

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

        int rc = sqlite3_prepare_v2(db, insert[table_id - 1], -1, &res, 0);
        if (rc != SQLITE_OK) {
            printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return -4;
        }

        for (int i = 1; i < col_number[table_id - 1]; i++)
            sqlite3_bind_text(res, i, values[i - 1], -1, NULL);

        if (table_id == 4) {
            if (!check_cargo_weight(db, atoi(values[2]), atoi(values[4])))
            {
                printf("Cargo weight can't be bigger then car capacity.\n");
                return 0;
            }
        }

        while (sqlite3_step(res) == SQLITE_ROW);
        break;
    }
    case 2: {

        printf("Enter id of updating row: ");
        int id;
        scanf("%d", &id);

        char values[6][100];
        printf("Enter values:\n");
        getchar();
        for (int i = 1; i < col_number[table_id - 1]; i++) {
            printf("\t%s: ", col_names[table_id - 1][i]);
            scanf("%99[^\n]", values[i - 1]);
            while (getchar() != '\n');
        }

        int rc = sqlite3_prepare_v2(db, update[table_id - 1], -1, &res, 0);
        if (rc != SQLITE_OK) {
            printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return -4;
        }

        for (int i = 1; i < col_number[table_id - 1]; i++)
            sqlite3_bind_text(res, i, values[i - 1], -1, NULL);
        sqlite3_bind_int(res, col_number[table_id - 1], id);

        if (table_id == 4) {
            if (!check_cargo_weight(db, atoi(values[2]), atoi(values[4])))
            {
                printf("Cargo weight can't be bigger then car capacity.\n");
                return 0;
            }
        }

        while (sqlite3_step(res) == SQLITE_ROW);
        break;
    }
    case 3: {
        int rc = sqlite3_prepare_v2(db, delete[table_id - 1], -1, &res, 0);
        if (rc != SQLITE_OK) {
            printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return -5;
        }

        printf("Enter id: ");
        int id;
        scanf("%d", &id);

        sqlite3_bind_int(res, 1, id);

        while (sqlite3_step(res) == SQLITE_ROW);
        break;
    }
    default:
        printf("Invalid action id.\n");
        return -3;
    }

    if(res != 0)
        sqlite3_finalize(res);

    return 0;
}
