#include <stdio.h>
#include <stdlib.h>

int main() {
    int rc;

    // 1. รันโปรแกรมหลักด้วย input script
    rc = system("../crm < tests/e2e_input.txt > tests/e2e_output.txt");
    if (rc != 0) {
        puts("[E2E] FAIL: crm execution error");
        return 1;
    }

    // 2. ตรวจสอบผลลัพธ์จากไฟล์ output
    if (system("grep -q '✓ Added.' tests/e2e_output.txt") != 0) {
        puts("[E2E] Add FAIL"); return 1;
    }

    if (system("grep -q '× Invalid phone' tests/e2e_output.txt") != 0) {
        puts("[E2E] Invalid Phone FAIL"); return 1;
    }

    if (system("grep -q 'Charlie' tests/e2e_output.txt") != 0) {
        puts("[E2E] Search/List FAIL"); return 1;
    }

    if (system("grep -q '0822222222' tests/e2e_output.txt") != 0) {
        puts("[E2E] Update FAIL"); return 1;
    }

    if (system("grep -q 'Unknown field' tests/e2e_output.txt") != 0) {
        puts("[E2E] Update Invalid Field FAIL"); return 1;
    }

    if (system("grep -q 'Marked Inactive' tests/e2e_output.txt") != 0) {
        puts("[E2E] Delete FAIL"); return 1;
    }

    if (system("grep -q '\\[Inactive only\\]' tests/e2e_output.txt") != 0) {
        puts("[E2E] List Inactive FAIL"); return 1;
    }

    if (system("grep -q '✓ Restored' tests/e2e_output.txt") != 0) {
        puts("[E2E] Restore FAIL"); return 1;
    }

    if (system("grep -q 'No active records found.' tests/e2e_output.txt") != 0) {
        puts("[E2E] Delete Cancel FAIL"); return 1;
    }

    if (system("grep -q 'Running unit tests' tests/e2e_output.txt") != 0) {
        puts("[E2E] Run Unit Test FAIL"); return 1;
    }

    if (system("grep -q 'Running E2E test' tests/e2e_output.txt") != 0) {
        puts("[E2E] Run E2E Test Menu FAIL"); return 1;
    }

    if (system("grep -q 'Bye!' tests/e2e_output.txt") != 0) {
        puts("[E2E] Exit FAIL"); return 1;
    }

    puts("[E2E] All tests passed!");
    return 0;
}
