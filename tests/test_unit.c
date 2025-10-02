#include <stdio.h>
#include <assert.h>
#include <string.h>

// รวมโค้ด customer_manager.c เข้ามาเพื่อเรียกใช้ฟังก์ชันโดยตรง
#include "../customer_manager.c"

// ===== Helper =====
static void reset_db(const char *path) {
    set_data_path(path);
    open_file();
    db.count = 0;
    save_csv();
    assert(db.count == 0);
    printf("[Reset] DB at %s cleared.\n", path);
}

// ===== Tests =====

// --- ADD USER ---
void test_add_valid() {
    printf("[Unit] test_add_valid...\n");
    reset_db("tests/test_unit.csv");

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
    assert(strcmp(db.items[0].status, "Active") == 0);
}

void test_add_invalid_phone() {
    printf("[Unit] test_add_invalid_phone...\n");
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "BadCo\nBadPerson\n123\nbad@test.com\n"); // เบอร์สั้นเกิน
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    add_user();
    assert(db.count == 0); // ไม่ควรเพิ่ม
}

void test_add_invalid_email() {
    printf("[Unit] test_add_invalid_email...\n");
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "BadCo\nBadPerson\n0901111111\nbademail\n"); // ไม่มี @
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    add_user();
    assert(db.count == 0);
}

void test_add_no_contact() {
    printf("[Unit] test_add_no_contact...\n");
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "NoContactCo\nNoPerson\n\n\n"); // ไม่มี phone + email
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    add_user();
    assert(db.count == 0);
}

// --- SEARCH ---
void test_search_found() {
    printf("[Unit] test_search_found...\n");

    reset_db("tests/test_unit.csv");
    // เตรียม 1 record
    strcpy(db.items[0].company, "SearchCo");
    strcpy(db.items[0].contact, "SearchPerson");
    strcpy(db.items[0].phone, "0902222222");
    strcpy(db.items[0].email, "search@test.com");
    strcpy(db.items[0].status, "Active");
    db.count = 1;
    save_csv();

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "SearchCo\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    search_user(); // ต้องเจอ
}

void test_search_not_found() {
    printf("[Unit] test_search_not_found...\n");
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "Nothing\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    search_user(); // ไม่ควรเจอ
}

// --- EDIT USER ---
void test_edit_valid() {
    printf("[Unit] test_edit_valid...\n");

    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company, "EditCo");
    strcpy(db.items[0].contact, "EditPerson");
    strcpy(db.items[0].phone, "0903333333");
    strcpy(db.items[0].email, "edit@test.com");
    strcpy(db.items[0].status, "Active");
    db.count = 1;
    save_csv();

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "EditCo\nA\nPhoneNumber\n0911111111\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    edit_user();
    assert(strcmp(db.items[0].phone, "0911111111") == 0);
}

void test_edit_invalid_field() {
    printf("[Unit] test_edit_invalid_field...\n");

    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company, "EditCo");
    strcpy(db.items[0].contact, "EditPerson");
    strcpy(db.items[0].phone, "0904444444");
    strcpy(db.items[0].email, "edit@test.com");
    strcpy(db.items[0].status, "Active");
    db.count = 1;
    save_csv();

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "EditCo\nA\nInvalidField\nsomething\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    edit_user(); // ควรไม่อัปเดต
    assert(strcmp(db.items[0].phone, "0904444444") == 0);
}

// --- DELETE/RESTORE ---
void test_delete_restore() {
    printf("[Unit] test_delete_restore...\n");

    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company, "DelCo");
    strcpy(db.items[0].contact, "DelPerson");
    strcpy(db.items[0].phone, "0905555555");
    strcpy(db.items[0].email, "del@test.com");
    strcpy(db.items[0].status, "Active");
    db.count = 1;
    save_csv();

    // delete
    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "DelCo\nA\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    delete_user();
    assert(strcmp(db.items[0].status, "Inactive") == 0);

    // restore
    f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "DelCo\nA\ny\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    restore_user();
    assert(strcmp(db.items[0].status, "Active") == 0);
}

void test_delete_not_found() {
    printf("[Unit] test_delete_not_found...\n");
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "Nope\n");
    fclose(f);
    freopen("tests/mock_input.txt", "r", stdin);

    delete_user(); // ไม่ควรลบอะไร
}

// ===== MAIN =====
int main() {
    printf("===== Running Unit Tests =====\n");

    // Add
    test_add_valid();
    test_add_invalid_phone();
    test_add_invalid_email();
    test_add_no_contact();

    // Search
    test_search_found();
    test_search_not_found();

    // Edit
    test_edit_valid();
    test_edit_invalid_field();

    // Delete/Restore
    test_delete_restore();
    test_delete_not_found();

    printf("===== All Unit Tests Passed =====\n");
    return 0;
}
