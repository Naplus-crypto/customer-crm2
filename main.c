// main.c
#include <stdio.h>

// ประกาศฟังก์ชันจาก customer_manager.c
void set_data_path(const char* path);
void open_file(void);
void display_menu(void);

int main(void) {
    // ตั้ง path สำหรับ CSV
    set_data_path("customers.csv");

    // โหลดข้อมูลลูกค้าจากไฟล์
    open_file();

    // เรียกเมนูหลัก
    display_menu();

    return 0;
}
