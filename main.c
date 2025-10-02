// main.c — entry point ของ Customer CRM
// Compile: gcc main.c customer_manager.c -std=c11 -O2 -Wall -Wextra -pedantic -o crm

#include <stdio.h>

// Forward declarations จาก customer_manager.c
void set_data_path(const char* path);
void open_file(void);
void display_menu(void);

int main(int argc, char *argv[]) {
    // สามารถกำหนด path ของ CSV ผ่าน argument ได้ เช่น ./crm data.csv
    if (argc > 1) {
        set_data_path(argv[1]);
    } else {
        set_data_path("customers.csv");
    }

    // โหลดข้อมูลจากไฟล์ (สร้างถ้ายังไม่มี)
    open_file();

    // เรียกเมนูหลัก
    display_menu();

    return 0;
}
