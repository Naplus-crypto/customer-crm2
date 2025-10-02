#include <stdio.h>
#include <assert.h>
#include <string.h>

// รวมโค้ด customer_manager.c เพื่อเรียกฟังก์ชันได้ตรง ๆ
#include "../customer_manager.c"

void test_add_user() {
    printf("[Unit] test_add_user...\n");

    set_data_path("tests/test_unit.csv");
    open_file();
    db.count = 0;
    save_csv();

    // mock input
    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "UnitCo\nUnitPerson\n0900000000\nunit@test.com\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    add_user();
    assert(db.count == 1);
    assert(strcmp(db.items[0].company, "UnitCo") == 0);
    assert(strcmp(db.items[0].contact, "UnitPerson") == 0);
    assert(strcmp(db.items[0].phone, "0900000000") == 0);
    assert(strcmp(db.items[0].email, "unit@test.com") == 0);
    assert(db.items[0].active == 1);
}

void test_edit_user() {
    printf("[Unit] test_edit_user...\n");

    // mock input สำหรับแก้ไข phone
    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "UnitCo\nA\nPhoneNumber\n0911111111\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    edit_user();
    assert(strcmp(db.items[0].phone, "0911111111") == 0);
}

void test_delete_restore_user() {
    printf("[Unit] test_delete_restore_user...\n");

    // mock input สำหรับ delete
    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "UnitCo\nA\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    delete_user();
    assert(db.items[0].active == 0);

    // mock input สำหรับ restore
    f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "UnitCo\nA\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    restore_user();
    assert(db.items[0].active == 1);
}

void test_search_user() {
    printf("[Unit] test_search_user...\n");

    // mock input สำหรับ search
    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "UnitCo\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    search_user(); // ควรเจอ record ที่มี UnitCo
}

int main() {
    printf("===== Running Unit Tests =====\n");
    test_add_user();
    test_edit_user();
    test_delete_restore_user();
    test_search_user();
    printf("[Unit] All tests passed!\n");
    return 0;
}
