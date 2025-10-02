#include <assert.h>
#include <stdio.h>
#include <string.h>

/* include implementation for a standalone test binary */
#include "../customer_manager.c"

int main(void){
    /* ใช้ไฟล์แยกสำหรับยูนิตเทสต์ */
    set_data_path("tests/customers_test.csv");
    open_file();
    int start = db.count;

    /* 1) เพิ่มบริษัทมี comma + email อย่างเดียว (phone ว่าง) */
    Customer a = {"ACME Co., Ltd.","Alice Ray","","alice@acme.test","Active"};
    assert(valid_company(a.company));
    assert(valid_contact(a.contact));
    assert(a.phone[0]=='\0');             /* optional */
    assert(valid_email_strict(a.email));
    assert(!is_duplicate(&a,-1));

    db.items[db.count++] = a;
    save_csv();

    /* 2) โหลดใหม่แล้วต้องอ่านค่าที่มี quote/comma ได้ถูก */
    open_file();
    int found=0;
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].company,"ACME Co., Ltd.")==0 &&
           strcmp(db.items[i].contact,"Alice Ray")==0 &&
           strcmp(db.items[i].email,"alice@acme.test")==0){
            found=1;
            break;
        }
    }
    assert(found==1);

    /* 3) เพิ่ม contact เดิมอีกเรคคอร์ด แต่เป็น phone อย่างเดียว (email ว่าง) */
    Customer b = {"ACME Co., Ltd.","Alice Ray","0812345678","", "Active"};
    assert(valid_company(b.company));
    assert(valid_contact(b.contact));
    assert(valid_phone_strict(b.phone));
    assert(b.email[0]=='\0');
    assert(!is_duplicate(&b,-1));   /* ไม่ซ้ำ: email ว่าง ใช้คีย์ (Company,Contact,Phone) */
    db.items[db.count++] = b;
    save_csv();

    /* 4) กัน duplicate ตามกฎ */
    Customer dup1 = {"ACME Co., Ltd.","Alice Ray","","alice@acme.test","Active"};
    assert(is_duplicate(&dup1,-1)); /* อีเมลเดียวกัน => duplicate */

    Customer dup2 = {"ACME Co., Ltd.","Alice Ray","0812345678","","Active"};
    assert(is_duplicate(&dup2,-1)); /* เบอร์เดียวกัน (เมื่อ email ว่าง) => duplicate */

    printf("✓ ALL UNIT TESTS PASSED\n");
    return 0;
}
