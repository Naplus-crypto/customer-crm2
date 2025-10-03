#include <stdio.h>
#include <assert.h>
#include <string.h>

/* FIX include path: customer_manager.c อยู่ root เดียวกัน */
#include "customer_manager.c"

/* Helper: reset DB & bind test csv */
static void reset_db(const char *path) {
    set_data_path(path);
    open_file();
    db.count = 0;
    /* เซฟ header ว่าง ๆ */
    FILE *f = fopen(path, "w");
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email,Status\n");
    fclose(f);
    open_file();
    assert(db.count == 0);
}

/* ---------- ADD ---------- */
static void test_add_valid() {
    reset_db("tests/test_unit.csv");
    FILE *f = fopen("tests/mock_input.txt", "w");
    fprintf(f, "UnitCo\nUnitPerson\n0900000000\nunit@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);

    add_user();
    assert(db.count==1);
    assert(strcmp(db.items[0].company,"UnitCo")==0);
    assert(strcmp(db.items[0].contact,"UnitPerson")==0);
    assert(strcmp(db.items[0].phone,"0900000000")==0);
    assert(strcmp(db.items[0].email,"unit@test.com")==0);
    assert(strcmp(db.items[0].status,"Active")==0);
}

static void test_add_invalid_company_contact(){
    reset_db("tests/test_unit.csv");
    FILE *f = fopen("tests/mock_input.txt","w");
    fprintf(f,"12345\nNoAlpha\n0900000000\nna@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user();                 /* company ไม่มีตัวอักษร */
    assert(db.count==0);

    f = fopen("tests/mock_input.txt","w");
    fprintf(f,"ValidCo\n123456\n0900000000\nc@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user();                 /* contact ไม่มีตัวอักษร */
    assert(db.count==0);
}

static void test_add_invalid_phone(){
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"BadCo\nBadPerson\n123\nbad@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user();
    assert(db.count==0);
}
static void test_add_invalid_email(){
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"BadCo\nBadPerson\n0901111111\nbademail\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user();
    assert(db.count==0);
}
static void test_add_no_contact(){
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"NoContactCo\nNoPerson\n\n\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user();
    assert(db.count==0);
}
static void test_add_phone_or_email_only(){
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"PhoneCo\nPhonePerson\n0812345678\n\n"); /* phone only */
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user(); assert(db.count==1);

    f=fopen("tests/mock_input.txt","w");
    fprintf(f,"EmailCo\nEmailPerson\n\nemail@test.com\n"); /* email only */
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user(); assert(db.count==2);
}
static void test_add_duplicate(){
    reset_db("tests/test_unit.csv");
    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"DupCo\nDupPerson\n0811111111\ndup@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user(); assert(db.count==1);

    f=fopen("tests/mock_input.txt","w");
    fprintf(f,"DupCo\nDupPerson\n0811111111\ndup@test.com\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    add_user(); assert(db.count==1);
}

/* ---------- SEARCH ---------- */
static void test_search_case_insensitive(){
    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company,"SearchCo");
    strcpy(db.items[0].contact,"SearchPerson");
    strcpy(db.items[0].phone,"0902222222");
    strcpy(db.items[0].email,"search@test.com");
    strcpy(db.items[0].status,"Active");
    db.count=1; /* ไม่ลืม set count */
    save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"searchco\n");  /* lowercase */
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    search_user(); /* แค่ตรวจว่าไม่ crash */
}

/* ---------- EDIT ---------- */
static void test_edit_duplicate_prevention(){
    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company,"EditCo");
    strcpy(db.items[0].contact,"PersonA");
    strcpy(db.items[0].phone,"0900000001");
    strcpy(db.items[0].email,"a@test.com");
    strcpy(db.items[0].status,"Active");
    strcpy(db.items[1].company,"EditCo");
    strcpy(db.items[1].contact,"PersonB");
    strcpy(db.items[1].phone,"0900000002");
    strcpy(db.items[1].email,"b@test.com");
    strcpy(db.items[1].status,"Active");
    db.count=2; save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"PersonB\nA\nContactPerson\nPersonA\ny\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    edit_user(); /* ไม่ควรยอมเปลี่ยนเพราะ duplicate */
    assert(strcmp(db.items[1].contact,"PersonB")==0);
}

/* Invalid phone/email while update */
static void test_edit_invalid_phone_email(){
    reset_db("tests/test_unit.csv");
    strcpy(db.items[0].company,"UCo");
    strcpy(db.items[0].contact,"UPer");
    strcpy(db.items[0].phone,"0903333333");
    strcpy(db.items[0].email,"u@test.com");
    strcpy(db.items[0].status,"Active");
    db.count=1; save_csv();

    FILE *f=fopen("tests/mock_input.txt","w");
    fprintf(f,"UCo\nA\nPhoneNumber\n123\ny\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    edit_user(); /* ควรไม่เปลี่ยน */
    assert(strcmp(db.items[0].phone,"0903333333")==0);

    f=fopen("tests/mock_input.txt","w");
    fprintf(f,"UCo\nA\nEmail\nbadmail\ny\n");
    fclose(f); freopen("tests/mock_input.txt","r",stdin);
    edit_user(); /* ควรไม่เปลี่ยน */
    assert(strcmp(db.items[0].email,"u@test.com")==0);
}

/* ---------- DELETE/RESTORE ---------- */
static void test_delete_restore_multiple(){
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

/* ---------- CSV escape round-trip ---------- */
static void test_csv_escape(){
    reset_db("tests/test_unit.csv");
    /* เขียน 1 แถวที่มี , และ " */
    FILE *f=fopen("tests/test_unit.csv","w");
    fprintf(f,"CompanyName,ContactPerson,PhoneNumber,Email,Status\n");
    fprintf(f,"\"ACME, Inc.\",\"John \"\"JJ\"\" Doe\",0812345678,jj@acme.com,Active\n");
    fclose(f);

    open_file();
    assert(db.count==1);
    assert(strcmp(db.items[0].company,"ACME, Inc.")==0);
    assert(strcmp(db.items[0].contact,"John \"JJ\" Doe")==0);
}

/* ---------- MAIN ---------- */
int main(){
    printf("===== Running Unit Tests =====\n");
    test_add_valid();
    test_add_invalid_company_contact();
    test_add_invalid_phone();
    test_add_invalid_email();
    test_add_no_contact();
    test_add_phone_or_email_only();
    test_add_duplicate();
    test_search_case_insensitive();
    test_edit_duplicate_prevention();
    test_edit_invalid_phone_email();
    test_delete_restore_multiple();
    test_csv_escape();
    printf("===== All Unit Tests Passed =====\n");
    return 0;
}
