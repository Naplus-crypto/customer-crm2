#include <assert.h>
#include <stdio.h>
#include <string.h>
#define TEMP_PATH "data/test_customers.csv"

#define INCLUDE_IMPL
#include "../customer_manager.c"

static void reset_test_csv(void){
    FILE *f = fopen(TEMP_PATH, "w");
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email\n");
    fprintf(f, "Tech Solutions,John Doe,555-1234,john@techsol.com\n");
    fclose(f);
}

void test_add_and_search(){
    set_data_path(TEMP_PATH);
    open_file();
    // mock stdin ด้วยชุดข้อมูลสั้น ๆ: เพิ่มลูกค้า 1 ราย
    // (ทดสอบผ่านการเรียกฟังก์ชันโดยตรงแทนการอ่านจริงจาก stdin)
    // เรียก add_user แบบ manual: จำลองด้วยการเพิ่มตรงลง db แล้ว save
    // (เพื่อให้ unit test โฟกัส logic save/load มากกว่าการอ่านคีย์บอร์ด)
    Customer c = {"Rocket Co","Rita Ray","0812345678","rita@rocket.io"};
    db.items[db.count++] = c; save_csv(); load_csv();

    // ตรวจ
    int found=0;
    for(int i=0;i<db.count;i++) if(strstr(db.items[i].company,"Rocket")) found++;
    assert(found==1);
    printf("✓ add_and_search PASSED\n");
}

void test_update_and_delete(){
    set_data_path(TEMP_PATH);
    open_file();

    // update phone ของ Tech Solutions
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].company,"Tech Solutions")==0){
            strncpy(db.items[i].phone,"0890000000",63);
        }
    }
    save_csv(); load_csv();

    // ตรวจว่าอัปเดตแล้ว
    int ok=0;
    for(int i=0;i<db.count;i++) if(strcmp(db.items[i].phone,"0890000000")==0) ok=1;
    assert(ok==1);

    // ลบรายการที่อัปเดต
    int kept=0;
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].phone,"0890000000")==0) continue;
        if(i!=kept) db.items[kept]=db.items[i];
        kept++;
    }
    db.count=kept; save_csv(); load_csv();

    // ตรวจว่าถูกลบแล้ว
    ok=1;
    for(int i=0;i<db.count;i++) if(strcmp(db.items[i].phone,"0890000000")==0) ok=0;
    assert(ok==1);
    printf("✓ update_and_delete PASSED\n");
}

int main(void){
    reset_test_csv();
    test_add_and_search();
    test_update_and_delete();
    puts("✓ ALL TESTS PASSED");
    return 0;
}
