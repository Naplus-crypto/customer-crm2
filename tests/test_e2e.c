#include <stdio.h>
#include <stdlib.h>

void run_e2e_test(void){
    printf("Running E2E test...\n");
    system("./crm < tests/e2e_input.txt > tests/e2e_output.txt");
    printf("E2E test finished. Check tests/e2e_output.txt\n");
}
