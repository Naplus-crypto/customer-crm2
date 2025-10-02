#include <stdio.h>
#include <stdlib.h>

int main(void){
    system("echo '2\nTest Co\nTester One\n0891234567\ntest@test.com\n6\n' | ./crm > tests/e2e_output.txt");

    FILE *f = fopen("tests/e2e_output.txt","r");
    if(!f){ perror("open e2e_output.txt"); return 1; }

    char line[256]; int add_ok=0, bye_ok=0;
    while(fgets(line,sizeof(line),f)){
        if(strstr(line,"Added.")) add_ok=1;
        if(strstr(line,"Bye!")) bye_ok=1;
    }
    fclose(f);

    if(add_ok) printf("[E2E] Add OK\n"); else printf("[E2E] Add FAIL\n");
    if(bye_ok) printf("[E2E] Exit OK\n"); else printf("[E2E] Exit FAIL\n");

    return (add_ok && bye_ok)?0:1;
}
