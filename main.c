#include <stdio.h>

/* ใช้ประกาศจาก customer_manager.c */
void set_data_path(const char* path);
void open_file(void);
void display_menu(void);

int main(void){
    set_data_path("customers.csv");
    open_file();
    display_menu();
    return 0;
}
