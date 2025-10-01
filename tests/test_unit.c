#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ใช้โค้ดจริงในไบนารีเทสต์นี้ */
#include "../customer_manager.c"

int main(void){
    set_data_path("tests/customers_test.csv");
    open_file();

    int before = db.count;

    /* เพิ่มข้อมูล valid */
    Customer c = {"UnitCo","Tester","0812345678","test@unit.com"};
    db.items[db.count++] = c;
    save_csv();
    assert(db.count == before + 1);

    /* ตรวจค่าที่เพิ่ม */
    assert(strcmp(db.items[db.count-1].company, "UnitCo") == 0);
    assert(strcmp(db.items[db.count-1].contact, "Tester") == 0);

    /* ตรวจ validation เบื้องต้น */
    assert(!valid_phone("12345"));      /* สั้นเกิน */
    assert(!valid_email("abc@bad"));    /* ไม่มี . ตามหลังโดเมน */

    printf("✓ ALL UNIT TESTS PASSED\n");
    return 0;
}
