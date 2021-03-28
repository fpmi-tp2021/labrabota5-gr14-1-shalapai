#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../include/user_scenario.h"
#include "../include/admin_scenario.h"

int main()
{
    sqlite3* db;
    char* err_msg = 0;
    sqlite3_stmt* res = 0;

    int rc = sqlite3_open("../src/auto_park.db", &db);

    if (rc != SQLITE_OK) {
        printf("Can't open database: %s.\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    else {
        printf("Opened database successfully.\n");
    }

    int isAllRight = 1;
    while (isAllRight) {
        printf("Choose the action:\n\t1) Authentication\n\t2) Exit\n\taction: ");
        int action_id;
        scanf("%d", &action_id);

        int USER_ID = -1;
        switch (action_id) {
        case 1: {
            int authentication = 1;
            while (authentication) {
                char username[30];
                char password[30];

                printf("Enter username: ");
                scanf("%29s", username);
                printf("Enter password: ");
                scanf("%29s", password);


                if (sqlite3_prepare_v2(db, "select * from users", -1, &res, 0) != SQLITE_OK) {
                    printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
                    return -100;
                }
                int is_found = 0;
                while (sqlite3_step(res) == SQLITE_ROW) {
                    if (!strcmp(username, sqlite3_column_text(res, 1))) {
                        is_found = 1;
                        if (strcmp(password, sqlite3_column_text(res, 2))) {
                            printf("Login or password are incorrect. Try again.\n");
                            break;
                        }
                        USER_ID = sqlite3_column_int(res, 0);
                        authentication = 0;
                        break;
                    }
                }
                if (!is_found)
                    printf("No such user. Try again.\n");
            }

            if (!USER_ID)
                admin_access(db);
            else {
                user_access(db, USER_ID);
            }

            break;
        }
        case 2: {
            printf("End of program.\n");
            isAllRight = 0;
            break;
        }
        default:
            printf("Invalid action id.\n");
            break;
        }
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return 0;
}
