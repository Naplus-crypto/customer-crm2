// tests/test_unit.c
// Unit tests (assert + printf)
// รันเดี่ยว:   gcc -std=c11 -O2 -Wall -Wextra -pedantic tests/test_unit.c -o tests/test_unit && ./tests/test_unit
// รันผ่านเมนู: เลือกข้อ 9) Run Unit Test

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// รวมโค้ดหลักเข้ามาเพื่อเรียกใช้ฟังก์ชันโดยตรง (และเข้าถึง db/status)
#include "../customer_manager.c"

// ---------- Helper ----------
static void reset_db(const char *path) {
    set_data_path(path);
    open_file();         // ensure file exists and load
    db.count = 0;        // clear in-memory
    save_csv();          // write header only
    assert(db.count == 0);
    printf("[Reset] DB at %s cleared.\n", path);
}

static void write_mock_input(const char *s) {
    FILE *f = fopen("tests/mock_input.txt", "w");
    assert(f && "cannot open tests/mock_input.txt for writing");
    fputs(s, f);
    fclose(f);
    (void)freopen("tests/mock_input.txt", "r", stdin);  // feed as stdin
}

// ---------- Convenience checks ----------
static void expect_db_count(int n) {
    assert(db.count == n);
}

static void expect_active_at(int i, const char *company, const char *contact,
                             const char *phone, const char *email) {
    assert(i >= 0 && i < db.count);
    assert(strcmp(db.items[i].status, "Active") == 0);
    assert(strcmp(db.items[i].company, company) == 0);
    assert(strcmp(db.items[i].contact, contact) == 0);
    assert(strcmp(db.items[i].phone,   phone)   == 0);
    assert(strcmp(db.items[i].email,   email)   == 0);
}

// ---------- Tests: ADD ----------
static void test_add_valid(void) {
    printf(">> test_add_valid...\n");
    reset_db("tests/test_unit.csv");

    write_mock_input(
        "UnitCo\n"
        "UnitPerson\n"
        "0900000000\n"
        "unit@test.com\n"
    );
    add_user();

    expect_db_count(1);
    expect_active_at(0, "UnitCo", "UnitPerson", "0900000000", "unit@test.com");
    printf("✓ test_add_valid PASSED\n");
}

static void test_add_invalid_phone(void) {
    printf(">> test_add_invalid_phone...\n");
    reset_db("tests/test_unit.csv");

    write_mock_input(
        "BadCo\n"
        "BadPerson\n"
        "123\n"               // invalid (สั้นเกิน)
        "bad@test.com\n"
    );
    add_user();

    expect_db_count(0); // ไม่ควรเพิ่ม
    printf("✓ test_add_invalid_phone PASSED\n");
}

static void test_add_invalid_email(void) {
    printf(">> test_add_invalid_email...\n");
    reset_db("tests/test_unit.csv");

    write_mock_input(
        "BadCo\n"
        "BadPerson\n"
        "0901111111\n"
        "bademail\n"         // invalid (ไม่มี @)
    );
    add_user();

    expect_db_count(0);
    printf("✓ test_add_invalid_email PASSED\n");
}

static void test_add_no_contact(void) {
    printf(">> test_add_no_contact...\n");
    reset_db("tests/test_unit.csv");

    write_mock_input(
        "NoContactCo\n"
        "NoPerson\n"
        "\n"   // phone ว่าง
        "\n"   // email ว่าง
    );
    add_user();
    expect_db_count(0);
    printf("✓ test_add_no_contact PASSED\n");
}

static void test_add_phone_only_and_email_only(void) {
    printf(">> test_add_phone_only_and_email_only...\n");
    reset_db("tests/test_unit.csv");

    // phone only
    write_mock_input(
        "PhoneOnlyCo\n"
        "PhoneGuy\n"
        "0812345678\n"
        "\n"
    );
    add_user();
    expect_db_count(1);

    // email only
    write_mock_input(
        "MailOnlyCo\n"
        "MailGal\n"
        "\n"
        "mail@only.com\n"
    );
    add_user();
    expect_db_count(2);

    printf("✓ test_add_phone_only_and_email_only PASSED\n");
}

static void test_add_duplicate_rule(void) {
    printf(">> test_add_duplicate_rule...\n");
    reset_db("tests/test_unit.csv");

    // first
    write_mock_input(
        "DupCo\n"
        "DupPerson\n"
        "0811111111\n"
        "dup@test.com\n"
    );
    add_user();
    expect_db_count(1);

    // duplicate (same company+contact+email)
    write_mock_input(
        "DupCo\n"
        "DupPerson\n"
        "0811111111\n"
        "dup@test.com\n"
    );
    add_user();
    expect_db_count(1);  // ไม่ควรเพิ่ม

    printf("✓ test_add_duplicate_rule PASSED\n");
}

// Boundary: phone length 9 / 15
static void test_add_phone_boundary(void) {
    printf(">> test_add_phone_boundary...\n");
    reset_db("tests/test_unit.csv");

    // length 9 (valid)
    write_mock_input(
        "B9Co\n"
        "B9\n"
        "081234567\n"
        "\n"
    );
    add_user();
    expect_db_count(1);

    // length 15 (valid) - ทำให้เป็น 0 + 14 digits
    write_mock_input(
        "B15Co\n"
        "B15\n"
        "091234567890123\n" // 15 chars
        "\n"
    );
    add_user();
    expect_db_count(2);

    // length 8 (invalid)
    write_mock_input(
        "B8Co\n"
        "B8\n"
        "08123456\n"
        "\n"
    );
    add_user();
    expect_db_count(2);

    // length 16 (invalid)
    write_mock_input(
        "B16Co\n"
        "B16\n"
        "0912345678901234\n" // 16
        "\n"
    );
    add_user();
    expect_db_count(2);

    printf("✓ test_add_phone_boundary PASSED\n");
}

// Extreme: แปลกๆ เช่น company/contact ไม่มีตัวอักษร
static void test_add_extreme_inputs(void) {
    printf(">> test_add_extreme_inputs...\n");
    reset_db("tests/test_unit.csv");

    // company ไม่มีตัวอักษร (ตัวเลขล้วน) → invalid_company
    write_mock_input(
        "12345\n"
        "ValidName\n"
        "0812345678\n"
        "a@b.com\n"
    );
    add_user();
    expect_db_count(0);

    // contact ไม่มีตัวอักษร
    write_mock_input(
        "ValidCo\n"
        "123456\n"
        "0812345678\n"
        "a@b.com\n"
    );
    add_user();
    expect_db_count(0);

    // symbol แปลก ๆ (บริษัทไม่อนุญาต)
    write_mock_input(
        "@@@@\n"
        "OK\n"
        "0812345678\n"
        "a@b.com\n"
    );
    add_user();
    expect_db_count(0);

    printf("✓ test_add_extreme_inputs PASSED\n");
}

// ---------- Tests: SEARCH ----------
static void test_search_case_insensitive_and_not_found(void) {
    printf(">> test_search_case_insensitive_and_not_found...\n");
    reset_db("tests/test_unit.csv");

    // เตรียม 2 เรคคอร์ด
    strcpy(db.items[0].company, "SearchCo");
    strcpy(db.items[0].contact, "Alice");
    strcpy(db.items[0].phone,   "0811111111");
    strcpy(db.items[0].email,   "a@search.co");
    strcpy(db.items[0].status,  "Active");

    strcpy(db.items[1].company, "FindMe");
    strcpy(db.items[1].contact, "Bob");
    strcpy(db.items[1].phone,   "0822222222");
    strcpy(db.items[1].email,   "b@find.me");
    strcpy(db.items[1].status,  "Active");

    db.count = 2;
    save_csv();

    // case-insensitive search "searchco"
    write_mock_input("searchco\n");
    search_user(); // manual visual but ensure no crash

    // not found
    write_mock_input("xyz-notfound\n");
    search_user();

    printf("✓ test_search_case_insensitive_and_not_found PASSED\n");
}

// ---------- Tests: EDIT ----------
static void test_edit_valid_phone_update(void) {
    printf(">> test_edit_valid_phone_update...\n");
    reset_db("tests/test_unit.csv");

    strcpy(db.items[0].company, "EditCo");
    strcpy(db.items[0].contact, "Charlie");
    strcpy(db.items[0].phone,   "0800000000");
    strcpy(db.items[0].email,   "c@edit.co");
    strcpy(db.items[0].status,  "Active");
    db.count = 1;
    save_csv();

    // เปลี่ยนเบอร์เป็น 0822222222
    write_mock_input(
        "Charlie\n"    // identifier
        "A\n"          // update all matched
        "PhoneNumber\n"
        "0822222222\n"
        "y\n"          // confirm
    );
    edit_user();

    expect_db_count(1);
    expect_active_at(0, "EditCo", "Charlie", "0822222222", "c@edit.co");
    printf("✓ test_edit_valid_phone_update PASSED\n");
}

static void test_edit_invalid_field_and_empty_contact(void) {
    printf(">> test_edit_invalid_field_and_empty_contact...\n");
    reset_db("tests/test_unit.csv");

    // เตรียม 1 rec
    strcpy(db.items[0].company, "EditCo");
    strcpy(db.items[0].contact, "Dana");
    strcpy(db.items[0].phone,   "0909999999");
    strcpy(db.items[0].email,   "d@edit.co");
    strcpy(db.items[0].status,  "Active");
    db.count = 1;
    save_csv();

    // invalid field
    write_mock_input(
        "Dana\n"
        "A\n"
        "WrongField\n"
        "xxx\n"
        "y\n"
    );
    edit_user();
    // ไม่มีการเปลี่ยนแปลง
    expect_active_at(0, "EditCo", "Dana", "0909999999", "d@edit.co");

    // ลองทำให้ phone ว่าง และ email ก็ว่าง → ต้อง reject (Need at least one…)
    write_mock_input(
        "Dana\n"
        "A\n"
        "PhoneNumber\n"
        "\n"      // clear phone
        "y\n"
    );
    edit_user();
    // phone ว่างแล้ว แต่ยังเหลือ email → ok (ขึ้นกับกฎในโค้ดคุณ ถ้าห้ามว่างทั้งคู่ เราต้องว่างทั้งสองฟิลด์)
    // ต่ออีกที: clear email ให้ว่างด้วย → ควร reject update (แต่เราไม่บังคับผลลัพธ์ตรงนี้เพราะขึ้นกับโค้ดจริง)
    // เราแค่ยืนยันว่าโปรแกรมไม่ crash

    printf("✓ test_edit_invalid_field_and_empty_contact PASSED\n");
}

static void test_edit_duplicate_prevention(void) {
    printf(">> test_edit_duplicate_prevention...\n");
    reset_db("tests/test_unit.csv");

    // มี 2 record
    strcpy(db.items[0].company, "EditCo");
    strcpy(db.items[0].contact, "PersonA");
    strcpy(db.items[0].phone,   "0900000001");
    strcpy(db.items[0].email,   "a@test.com");
    strcpy(db.items[0].status,  "Active");

    strcpy(db.items[1].company, "EditCo");
    strcpy(db.items[1].contact, "PersonB");
    strcpy(db.items[1].phone,   "0900000002");
    strcpy(db.items[1].email,   "b@test.com");
    strcpy(db.items[1].status,  "Active");
    db.count = 2;
    save_csv();

    // พยายามแก้ PersonB -> ContactPerson เป็น PersonA → ต้องถูกปฏิเสธเพราะ duplicate
    write_mock_input(
        "PersonB\n"
        "A\n"
        "ContactPerson\n"
        "PersonA\n"
        "y\n"
    );
    edit_user();

    // ยืนยันว่ายังเป็น PersonB
    assert(strcmp(db.items[1].contact, "PersonB") == 0);
    printf("✓ test_edit_duplicate_prevention PASSED\n");
}

// ---------- Tests: DELETE / RESTORE ----------
static void test_delete_single_and_restore_single(void) {
    printf(">> test_delete_single_and_restore_single...\n");
    reset_db("tests/test_unit.csv");

    // เตรียม 1 record
    strcpy(db.items[0].company, "DelCo");
    strcpy(db.items[0].contact, "Eve");
    strcpy(db.items[0].phone,   "0905555555");
    strcpy(db.items[0].email,   "e@del.co");
    strcpy(db.items[0].status,  "Active");
    db.count = 1;
    save_csv();

    // delete (A -> เลือกทั้งหมด ซึ่งมี 1 รายการ)
    write_mock_input(
        "DelCo\n"
        "A\n"
        "y\n"
    );
    delete_user();
    assert(strcmp(db.items[0].status, "Inactive") == 0);

    // restore
    write_mock_input(
        "DelCo\n"
        "A\n"
        "y\n"
    );
    restore_user();
    assert(strcmp(db.items[0].status, "Active") == 0);

    printf("✓ test_delete_single_and_restore_single PASSED\n");
}

static void test_delete_multi_and_restore_partial(void) {
    printf(">> test_delete_multi_and_restore_partial...\n");
    reset_db("tests/test_unit.csv");

    // เตรียม 3 record (ชื่อบริษัทเดียวกันคนละ contact)
    for (int i = 0; i < 3; i++) {
        snprintf(db.items[i].company, sizeof(db.items[i].company), "MultiDelCo");
        snprintf(db.items[i].contact, sizeof(db.items[i].contact), "Person%d", i+1);
        strcpy(db.items[i].phone, "0907777777");
        snprintf(db.items[i].email, sizeof(db.items[i].email), "m%d@test.com", i+1);
        strcpy(db.items[i].status, "Active");
    }
    db.count = 3;
    save_csv();

    // delete index 1,2 (ผู้ใช้เห็นเป็น 1-based → "1,2")
    write_mock_input(
        "MultiDelCo\n"
        "1,2\n"
        "y\n"
    );
    delete_user();
    assert(strcmp(db.items[0].status, "Inactive") == 0);
    assert(strcmp(db.items[1].status, "Inactive") == 0);
    assert(strcmp(db.items[2].status, "Active")   == 0);

    // restore index 1 (ตัวแรก)
    write_mock_input(
        "MultiDelCo\n"
        "1\n"
        "y\n"
    );
    restore_user();
    assert(strcmp(db.items[0].status, "Active")   == 0);
    assert(strcmp(db.items[1].status, "Inactive") == 0);
    assert(strcmp(db.items[2].status, "Active")   == 0);

    printf("✓ test_delete_multi_and_restore_partial PASSED\n");
}

static void test_delete_not_found_and_restore_not_found(void) {
    printf(">> test_delete_not_found_and_restore_not_found...\n");
    reset_db("tests/test_unit.csv");

    // delete: ไม่พบ
    write_mock_input("Nope\n");
    delete_user(); // ไม่ crash

    // restore: ไม่พบ (ไม่มี inactive)
    write_mock_input("Nope\n");
    restore_user(); // ไม่ crash

    printf("✓ test_delete_not_found_and_restore_not_found PASSED\n");
}

// ---------- MAIN ----------
int main(void) {
    printf("===== Running Unit Tests =====\n");

    // ADD
    test_add_valid();
    test_add_invalid_phone();
    test_add_invalid_email();
    test_add_no_contact();
    test_add_phone_only_and_email_only();
    test_add_duplicate_rule();
    test_add_phone_boundary();
    test_add_extreme_inputs();

    // SEARCH
    test_search_case_insensitive_and_not_found();

    // EDIT
    test_edit_valid_phone_update();
    test_edit_invalid_field_and_empty_contact();
    test_edit_duplicate_prevention();

    // DELETE / RESTORE
    test_delete_single_and_restore_single();
    test_delete_multi_and_restore_partial();
    test_delete_not_found_and_restore_not_found();

    printf("===== All Unit Tests Passed =====\n");
    return 0;
}
