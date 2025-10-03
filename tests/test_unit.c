#include <stdio.h>
#include <assert.h>
#include <string.h>

// รวมโค้ด customers_manager.c
#include "customers_manager.c"

// ===== Helper =====
static void reset_db(const char *path) {
    set_data_path(path);
    open_file();
    db.count = 0;
    save_csv();
    assert(db.count == 0);
}

// --- ADD ---
void test_add_valid();
void test_add_invalids();
void test_add_duplicate();
void test_add_phone_or_email_only();

// --- SEARCH ---
void test_search_case_insensitive();

// --- EDIT ---
void test_edit_invalids();
void test_edit_duplicate_prevention();

// --- DELETE/RESTORE ---
void test_delete_restore_multiple();
void test_restore_not_found();

// --- EDGE CASE CSV ---
void test_csv_escape();

int main() {
    printf("===== Running Unit Tests =====\n");

    test_add_valid();
    test_add_invalids();
    test_add_phone_or_email_only();
    test_add_duplicate();

    test_search_case_insensitive();

    test_edit_invalids();
    test_edit_duplicate_prevention();

    test_delete_restore_multiple();
    test_restore_not_found();

    test_csv_escape();

    printf("===== All Unit Tests Passed =====\n");
    return 0;
}

// --- Implementations ---

void test_add_valid() {
    reset_db("tests/test_unit.csv");
    FILE *f = fopen("tests/mock_input.txt","w");
    fprintf(f,"Co1\nPerson1\n0901111111\np1@test.com\n");
    fclose(f);
    freopen("tests/mock_input.txt","r",stdin);
    add_user();
    assert(db.count==1);
}

void test_add_invalids() {
    reset_db("tests/test_unit.csv");
    FILE *f = fopen("tests/mock_input.txt","w");
    fprintf(f,"12345\nNoAlpha\n0900000000\nna@test.com\n"); // invalid company
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==0);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"ValidCo\n123456\n0900000000\nc@test.com\n"); // invalid contact
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==0);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"PhoneBad\nBad\n123\nbad@test.com\n"); // bad phone
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==0);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"EmailBad\nBad\n0901111111\nbademail\n"); // bad email
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==0);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"NoContact\nNC\n\n\n"); // no contact
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==0);
}

void test_add_phone_or_email_only() {
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt","w");
    fprintf(f,"PhoneCo\nP1\n0812345678\n\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==1);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"EmailCo\nE1\n\nmail@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==2);
}

void test_add_duplicate() {
    reset_db("tests/test_unit.csv");

    FILE *f = fopen("tests/mock_input.txt","w");
    fprintf(f,"DupCo\nDP\n0811111111\ndup@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==1);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"DupCo\nDP\n0811111111\ndup@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin); add_user();
    assert(db.count==1); // duplicate blocked
}

void test_search_case_insensitive() {
    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company,"SearchCo");
    strcpy(db.items[0].contact,"Person");
    strcpy(db.items[0].phone,"0909999999");
    strcpy(db.items[0].email,"s@test.com");
    strcpy(db.items[0].status,"Active");
    db.count=1; save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"searchco\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    search_user();
}

void test_edit_invalids() {
    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company,"EditCo");
    strcpy(db.items[0].contact,"EP");
    strcpy(db.items[0].phone,"0903333333");
    strcpy(db.items[0].email,"e@test.com");
    strcpy(db.items[0].status,"Active");
    db.count=1; save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"EditCo\nA\nPhoneNumber\n123\ny\n"); // invalid phone
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    edit_user();
    assert(strcmp(db.items[0].phone,"0903333333")==0);

    f=fopen("tests/mock_input.txt","w");
    fprintf(f,"EditCo\nA\nEmail\nbademail\ny\n"); // invalid email
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    edit_user();
    assert(strcmp(db.items[0].email,"e@test.com")==0);
}

void test_edit_duplicate_prevention() {
    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company,"EditCo");
    strcpy(db.items[0].contact,"A");
    strcpy(db.items[0].phone,"0900000001");
    strcpy(db.items[0].email,"a@test.com");
    strcpy(db.items[0].status,"Active");
    strcpy(db.items[1].company,"EditCo");
    strcpy(db.items[1].contact,"B");
    strcpy(db.items[1].phone,"0900000002");
    strcpy(db.items[1].email,"b@test.com");
    strcpy(db.items[1].status,"Active");
    db.count=2; save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"B\nA\nContactPerson\nA\ny\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    edit_user();
    assert(strcmp(db.items[1].contact,"B")==0);
}

void test_delete_restore_multiple() {
    reset_db("tests/test_unit.csv");
    for(int i=0;i<3;i++){
        sprintf(db.items[i].company,"MultiDelCo");
        sprintf(db.items[i].contact,"Person%d",i+1);
        strcpy(db.items[i].phone,"0907777777");
        sprintf(db.items[i].email,"m%d@test.com",i+1);
        strcpy(db.items[i].status,"Active");
    }
    db.count=3; save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"MultiDelCo\n1,2\ny\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    delete_user();
    assert(strcmp(db.items[0].status,"Inactive")==0);
    assert(strcmp(db.items[1].status,"Inactive")==0);
    assert(strcmp(db.items[2].status,"Active")==0);

    f=fopen("tests/mock_input.txt","w");
    fprintf(f,"MultiDelCo\n1\ny\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    restore_user();
    assert(strcmp(db.items[0].status,"Active")==0);
}

void test_restore_not_found() {
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"Nope\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    restore_user(); // ควรไม่ crash
}

void test_csv_escape() {
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"\"Quoted, Co\"\nQMan\n0912345678\nq@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user();
    assert(db.count==1);
}
