#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int rc = system("./crm < tests/e2e_input.txt > tests/e2e_output.txt");
    if (rc != 0) { puts("[E2E] FAIL: crm execution error"); return 1; }

    /* ADD & validation messages */
    if (system("grep -q '✓ Added\\.' tests/e2e_output.txt"))   { puts("[E2E] Add FAIL"); return 1; }
    if (system("grep -q '× Invalid phone' tests/e2e_output.txt")) { puts("[E2E] Invalid Phone FAIL"); return 1; }
    if (system("grep -q '× Invalid email' tests/e2e_output.txt")) { puts("[E2E] Invalid Email FAIL"); return 1; }
    if (system("grep -q '× Company ไม่ถูกต้อง' tests/e2e_output.txt")) { puts("[E2E] Invalid Company FAIL"); return 1; }
    if (system("grep -q '× Contact ไม่ถูกต้อง' tests/e2e_output.txt")) { puts("[E2E] Invalid Contact FAIL"); return 1; }
    if (system("grep -q '× ต้องมีอย่างน้อย 1 ช่องทางติดต่อ' tests/e2e_output.txt")) { puts("[E2E] Missing Contact FAIL"); return 1; }
    if (system("grep -q '× Duplicate:' tests/e2e_output.txt")) { puts("[E2E] Duplicate Add FAIL"); return 1; }

    /* Search/List */
    if (system("grep -q 'Charlie' tests/e2e_output.txt")) { puts("[E2E] Search/List FAIL"); return 1; }

    /* Update paths */
    if (system("grep -q '0822222222' tests/e2e_output.txt")) { puts("[E2E] Update FAIL"); return 1; }
    if (system("grep -q 'Unknown field' tests/e2e_output.txt")) { puts("[E2E] Update Invalid Field FAIL"); return 1; }
    if (system("grep -q 'Duplicate after update' tests/e2e_output.txt") !=0) { puts("[E2E] Update Duplicate Prevention FAIL"); return 1; }
    if (system("grep -q 'Need at least one of Phone/Email' tests/e2e_output.txt")) { puts("[E2E] Update Empty Contact FAIL"); return 1; }

    /* Delete / Inactive list */
    if (system("grep -q 'Marked Inactive' tests/e2e_output.txt")) { puts("[E2E] Delete FAIL"); return 1; }
    if (system("grep -q '\\[Inactive only\\]' tests/e2e_output.txt")) { puts("[E2E] List Inactive FAIL"); return 1; }

    /* Restore */
    if (system("grep -q '✓ Restored' tests/e2e_output.txt")) { puts("[E2E] Restore FAIL"); return 1; }

    /* Unit test call via menu appears */
    if (system("grep -q 'Running unit tests' tests/e2e_output.txt")) { puts("[E2E] Run Unit Test FAIL"); return 1; }

    /* Exit */
    if (system("grep -q 'Bye!' tests/e2e_output.txt")) { puts("[E2E] Exit FAIL"); return 1; }

    puts("[E2E] All tests passed!");
    return 0;
}
