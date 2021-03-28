#include <stdio.h>
#include "../include/user_scenario.h"
#include "../include/admin_scenario.h"

int user_access(sqlite3* db, int USER_ID)
{
    int isAllRight = 1;
    while (isAllRight)
    {
        printf("Choose the action:\n\t1) Show info\n\t2) Logout\naction: ");
        int action_id;
        scanf("%d", &action_id);

        switch (action_id)
        {
        case 1:
            if (select_db_user(db, USER_ID))
                isAllRight = 0;
            break;
        case 2:
            printf("Logout.\n");
            isAllRight = 0;
            break;
        default:
            printf("Wrong action_id.\n");
            isAllRight = 0;
            break;
        }
    }
    return isAllRight;
}

int your_salary_for_the_period(sqlite3* db, char* start_date, char* end_date, int USER_ID)
{
    char* query = "select drivers.surname, sum(orders.cost) * 0.2 as salary,\
                   min(orders.date) as start_date,\
                   max(orders.date) as end_date from drivers inner join orders on drivers.id=orders.driver_id\
                   where orders.date between ? and ? and drivers.id = ?;";

    sqlite3_stmt* res;
    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(res, 1, start_date, -1, NULL);
    sqlite3_bind_text(res, 2, end_date, -1, NULL);
    sqlite3_bind_int(res, 3, USER_ID);

    while (sqlite3_step(res) == SQLITE_ROW) {
        printf("\n***********************************************\n");
        for (int i = 0; i < 4; i++)
            printf("%s - %s\n", sqlite3_column_name(res, i), sqlite3_column_text(res, i));
        printf("\n***********************************************\n\n");
    }
    sqlite3_finalize(res);

    return 1;
}

int select_db_user(sqlite3* db, int USER_ID)
{
    char* query_name[] = {
        "a list of completed orders for the period",
        "total mileage and total weight of transported goods for each your vechicles",
        "total number of trips, total weight transported goods, the amount of money earned",
        "determines the amount of money for the specified period, credited to your for transportation."
    };

    char* queries[] = {
        "select * from orders where driver_id=? and date between ? and ?",
        "select distinct drivers.surname, vechicles.brand, sum(orders.mileage) as mileage,\
         sum(orders.cargo_weight) as cargo_weight from drivers, vechicles inner join orders on\
         drivers.id = orders.driver_id and vechicles.id = orders.car_id where drivers.id = ? group by orders.car_id; ",
        "select drivers.surname, count(*) as total_orders, sum(orders.cargo_weight) as total_weight,\
         sum(orders.cost) * 0.2 as earned from drivers inner join orders on drivers.id=orders.driver_id\
         where orders.driver_id = ?;"
    };

    int col_number[3] = { 7, 4, 4};

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
    query_id -= 1;


    if (query_id == 0) {
        sqlite3_stmt* res;
        int rc = sqlite3_prepare_v2(db, queries[query_id], -1, &res, 0);
        if (rc != SQLITE_OK) {
            printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return -4;
        }

        char date1[11];
        char date2[11];

        printf("Enter dates in format yyyy-mm-dd:\n\tstart date: ");
        scanf("%10s", date1);
        printf("\tend date: ");
        scanf("%10s", date2);

        sqlite3_bind_int(res, 1, USER_ID);
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
    else if (query_id == 3) {
        char date1[11];
        char date2[11];

        printf("Enter dates in format yyyy-mm-dd:\n\tstart date: ");
        scanf("%10s", date1);
        printf("\tend date: ");
        scanf("%10s", date2);

        if (!your_salary_for_the_period(db, date1, date2, USER_ID)) {
            printf("Cannot perform query.\n");
            return -7;
        }
    }
    else {
        sqlite3_stmt* res;
        int rc = sqlite3_prepare_v2(db, queries[query_id], -1, &res, 0);
        if (rc != SQLITE_OK) {
            printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return -5;
        }

        sqlite3_bind_int(res, 1, USER_ID);

        while (sqlite3_step(res) == SQLITE_ROW) {
            printf("\n***********************************************\n");
            for (int i = 0; i < col_number[query_id]; i++)
                printf("%s - %s\n", sqlite3_column_name(res, i), sqlite3_column_text(res, i));
            printf("\n***********************************************\n\n");
        }
        sqlite3_finalize(res);
    }

    return 0;
}