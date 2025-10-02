#include <stdio.h>
#include <stdlib.h>

/* from customer_manager.c */
void open_file(void);
void list_users(void);
void list_inactive(void);
void add_user(void);
void search_user(void);
void edit_user(void);
void delete_user(void);
void restore_user(void);
void run_unit_test(void);
void run_e2e_test(void);

int main(void){
    open_file();
    int choice=0; char line[32];
    for(;;){
        puts("==== Customer CRM ===");
        puts("1) List all");
        puts("2) Add");
        puts("3) Search");
        puts("4) Update field");
        puts("5) Delete (soft)");
        puts("6) Exit");
        puts("7) List inactive");
        puts("8) Restore");
        puts("9) Run Unit Test");
        puts("10) Run E2E Test");
        printf("Choose [1-10]: ");
        if(!fgets(line,sizeof(line),stdin)) break;
        choice = atoi(line);

        if      (choice==1)  list_users();
        else if (choice==2)  add_user();
        else if (choice==3)  search_user();
        else if (choice==4)  edit_user();
        else if (choice==5)  delete_user();
        else if (choice==6){ puts("Bye!"); break; }
        else if (choice==7)  list_inactive();
        else if (choice==8)  restore_user();
        else if (choice==9)  run_unit_test();
        else if (choice==10) run_e2e_test();
        else puts("Invalid choice.");
    }
    return 0;
}
