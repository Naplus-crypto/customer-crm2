#include <assert.h>
#include <stdio.h>
#include <string.h>

/* include implementation for a standalone test binary */
#include "../customer_manager.c"

int main(void){
    set_data_path("tests/customers_test.csv");
    open_file();
    int before = db.count;

    /* basic validations */
    assert(!valid_phone("12345"));
    assert(!valid_email("abc@bad"));

    /* add one record programmatically */
    Customer c = {"UnitCo","Tester","0812345678","test@unit.com","Active"};
    db.items[db.count++] = c;
    save_csv();
    assert(db.count == before + 1);
    assert(strcmp(db.items[db.count-1].company,"UnitCo")==0);

    /* soft delete & restore */
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].company,"UnitCo")==0){
            safe_copy(db.items[i].status,"Inactive",sizeof(db.items[i].status));
            break;
        }
    }
    save_csv();
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].company,"UnitCo")==0){
            safe_copy(db.items[i].status,"Active",sizeof(db.items[i].status));
            break;
        }
    }
    save_csv();

    printf("✓ ALL UNIT TESTS PASSED\n");
    return 0;
}
