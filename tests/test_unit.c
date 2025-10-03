#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../customer_manager.c"

/* ---------- Helpers ---------- */
static void reset_db(const char *path) {
    set_data_path(path);
    open_file();
    db.count = 0;
    save_csv();
    assert(db.count == 0);
    printf("[Reset] DB at %s cleared.\n", path);
}

/* เขียนสตริงลงไฟล์และผูกเป็น stdin อย่างปลอดภัย (ไม่มี warning) */
static void mock_stdin_from_string(const char *payload) {
    FILE *f = fopen("tests/mock_input.txt", "w");
    if (!f) { perror("fopen"); exit(1); }
    if (fputs(payload, f) == EOF) { perror("fputs"); fclose(f); exit(1); }
    if (fclose(f) == EOF) { perror("fclose"); exit(1); }
    if (!freopen("tests/mock_input.txt", "r", stdin)) { perror("freopen"); exit(1); }
}

/* ---------- Tests ---------- */

/* --- ADD USER --- */
static void test_add_valid(void) {
    printf("[Unit] test_add_valid...\n");
    reset_db("tests/test_unit.csv");
    mock_stdin_from_string(
        "UnitCo\n"
        "UnitPerson\n"
        "0900000000\n"
        "unit@test.com\n"
    );
    add_user();
    assert(db.count == 1);
    assert(strcmp(db.items[0].company, "UnitCo") == 0);
    assert(strcmp(db.items[0].contact, "UnitPerson") == 0);
    assert(strcmp(db.items[0].phone,   "0900000000") == 0);
    assert(strcmp(db.items[0].email,   "unit@test.com") == 0);
    assert(strcmp(db.items[0].status,  "Active") == 0);
}

static void test_add_invalid_company_contact(void) {
    printf("[Unit] test_add_invalid_company_contact...\n");
    reset_db("tests/test_unit.csv");

    /* company ไม่มีตัวอักษร */
    mock_stdin_from_string(
        "12345\n"
        "NoAlpha\n"
        "0900000000\n"
        "na@test.com\n"
    );
    add_user();
    assert(db.count == 0);

    /* contact ไม่มีตัวอักษร */
    mock_stdin_from_string(
        "ValidCo\n"
        "123456\n"
        "0900000000\n"
        "c@test.com\n"
    );
    add_user();
    assert(db.count == 0);
}

static void test_add_invalid_phone(void) {
    printf("[Unit] test_add_invalid_phone...\n");
    reset_db("tests/test_unit.csv");
    mock_stdin_from_string(
        "BadCo\n"
        "BadPerson\n"
        "123\n"
        "bad@test.com\n"
    );
    add_user();
    assert(db.count == 0);
}

static void test_add_invalid_email(void) {
    printf("[Unit] test_add_invalid_email...\n");
    reset_db("tests/test_unit.csv");
    mock_stdin_from_string(
        "BadCo\n"
        "BadPerson\n"
        "0901111111\n"
        "bademail\n"
    );
    add_user();
    assert(db.count == 0);
}

static void test_add_no_contact(void) {
    printf("[Unit] test_add_no_contact...\n");
    reset_db("tests/test_unit.csv");
    mock_stdin_from_string(
        "NoContactCo\n"
        "NoPerson\n"
        "\n"
        "\n"
    );
    add_user();
    assert(db.count == 0);
}

static void test_add_phone_or_email_only(void) {
    printf("[Unit] test_add_phone_or_email_only...\n");
    reset_db("tests/test_unit.csv");

    /* phone only */
    mock_stdin_from_string(
        "PhoneCo\n"
        "PhonePerson\n"
        "0812345678\n"
        "\n"
    );
    add_user();
    assert(db.count == 1);

    /* email only */
    mock_stdin_from_string(
        "EmailCo\n"
        "EmailPerson\n"
        "\n"
        "email@test.com\n"
    );
    add_user();
    assert(db.count == 2);
}

static void test_add_duplicate(void) {
    printf("[Unit] test_add_duplicate...\n");
    reset_db("tests/test_unit.csv");
    mock_stdin_from_string(
        "DupCo\n"
        "DupPerson\n"
        "0811111111\n"
        "dup@test.com\n"
    );
    add_user();
    assert(db.count == 1);

    /* add ซ้ำ */
    mock_stdin_from_string(
        "DupCo\n"
        "DupPerson\n"
        "0811111111\n"
        "dup@test.com\n"
    );
    add_user();
    assert(db.count == 1);
}

/* --- SEARCH --- */
static void test_search_case_insensitive(void) {
    printf("[Unit] test_search_case_insensitive...\n");
    reset_db("tests/test_unit.csv");

    strcpy(db.items[0].company, "SearchCo");
    strcpy(db.items[0].contact, "SearchPerson");
    strcpy(db.items[0].phone,   "0902222222");
    strcpy(db.items[0].email,   "search@test.com");
    strcpy(db.items[0].status,  "Active");
    db.count = 1;
    save_csv();

    mock_stdin_from_string("searchco\n");
    search_user(); /* ต้องเจอ */
}

/* --- EDIT USER --- */
static void test_edit_duplicate_prevention(void) {
    printf("[Unit] test_edit_duplicate_prevention...\n");
    reset_db("tests/test_unit.csv");

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

    /* จะพยายามเปลี่ยน PersonB -> PersonA (ชนซ้ำ) */
    mock_stdin_from_string(
        "PersonB\n"
        "A\n"
        "ContactPerson\n"
        "PersonA\n"
        "y\n"
    );
    edit_user();
    assert(strcmp(db.items[1].contact, "PersonB") == 0);
}

static void test_edit_invalid_phone_email(void) {
    printf("[Unit] test_edit_invalid_phone_email...\n");
    reset_db("tests/test_unit.csv");

    strcpy(db.items[0].company, "XCo");
    strcpy(db.items[0].contact, "X");
    strcpy(db.items[0].phone,   "0912345678");
    strcpy(db.items[0].email,   "x@test.com");
    strcpy(db.items[0].status,  "Active");
    db.count = 1;
    save_csv();

    /* เปลี่ยน phone เป็นค่าไม่ถูกต้อง -> ต้องไม่เปลี่ยน */
    mock_stdin_from_string(
        "XCo\n"
        "A\n"
        "PhoneNumber\n"
        "12\n"
        "y\n"
    );
    edit_user();
    assert(strcmp(db.items[0].phone, "0912345678") == 0);

    /* เปลี่ยน email เป็นค่าไม่ถูกต้อง -> ต้องไม่เปลี่ยน */
    mock_stdin_from_string(
        "XCo\n"
        "A\n"
        "Email\n"
        "bad\n"
        "y\n"
    );
    edit_user();
    assert(strcmp(db.items[0].email, "x@test.com") == 0);
}

/* --- DELETE / RESTORE --- */
static void test_delete_restore_multiple(void) {
    printf("[Unit] test_delete_restore_multiple...\n");
    reset_db("tests/test_unit.csv");

    for (int i = 0; i < 3; i++) {
        sprintf(db.items[i].company, "MultiDelCo");
        sprintf(db.items[i].contact, "Person%d", i+1);
        strcpy(db.items[i].phone, "0907777777");
        sprintf(db.items[i].email, "m%d@test.com", i+1);
        strcpy(db.items[i].status, "Active");
    }
    db.count = 3;
    save_csv();

    /* delete index 1,2 */
    mock_stdin_from_string(
        "MultiDelCo\n"
        "1,2\n"
        "y\n"
    );
    delete_user();
    assert(strcmp(db.items[0].status, "Inactive") == 0);
    assert(strcmp(db.items[1].status, "Inactive") == 0);
    assert(strcmp(db.items[2].status, "Active") == 0);

    /* restore index 1 */
    mock_stdin_from_string(
        "MultiDelCo\n"
        "1\n"
        "y\n"
    );
    restore_user();
    assert(strcmp(db.items[0].status, "Active") == 0);
}

int main(void) {
    printf("===== Running Unit Tests =====\n");

    /* Add */
    test_add_valid();
    test_add_invalid_company_contact();
    test_add_invalid_phone();
    test_add_invalid_email();
    test_add_no_contact();
    test_add_phone_or_email_only();
    test_add_duplicate();

    /* Search */
    test_search_case_insensitive();

    /* Edit */
    test_edit_duplicate_prevention();
    test_edit_invalid_phone_email();

    /* Delete/Restore */
    test_delete_restore_multiple();

    printf("===== All Unit Tests Passed =====\n");
    return 0;
}
