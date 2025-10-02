#include <stdio.h>
#include <stdlib.h>

int main() {
    // รันโปรแกรมหลัก โดยใช้ input จำลอง แล้วเก็บ output ลงไฟล์
    system("../crm < tests/e2e_input.txt > tests/e2e_output.txt");

    // ตรวจสอบผลลัพธ์ด้วย grep
    if(system("grep -q 'Added.' tests/e2e_output.txt") != 0) {
        puts("[E2E] Add FAIL"); return 1;
    }
    if(system("grep -q 'Rocket Co' tests/e2e_output.txt") != 0) {
        puts("[E2E] List FAIL"); return 1;
    }
    if(system("grep -q 'Updated' tests/e2e_output.txt") != 0) {
        puts("[E2E] Update FAIL"); return 1;
    }
    if(system("grep -q '0890000000' tests/e2e_output.txt") != 0) {
        puts("[E2E] Search After Update FAIL"); return 1;
    }
    if(system("grep -q 'Inactive' tests/e2e_output.txt") != 0) {
        puts("[E2E] Delete/List Inactive FAIL"); return 1;
    }
    if(system("grep -q 'Restored' tests/e2e_output.txt") != 0) {
        puts("[E2E] Restore FAIL"); return 1;
    }
    if(system("grep -q 'Bye!' tests/e2e_output.txt") != 0) {
        puts("[E2E] Exit FAIL"); return 1;
    }

    puts("[E2E] All tests passed!");
    return 0;
}
