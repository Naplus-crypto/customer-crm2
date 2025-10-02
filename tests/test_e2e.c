#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void){
    int rc = system("./crm < tests/e2e_input.txt > tests/e2e_output.txt");
    if(rc!=0){ fprintf(stderr,"runner failed\n"); return 1; }

    FILE *f=fopen("tests/e2e_output.txt","r");
    if(!f){ perror("open e2e_output.txt"); return 1; }
    char line[512]; int ok_add=0, ok_upd=0, ok_del=0, ok_exit=0;
    while(fgets(line,sizeof(line),f)){
        if(strstr(line,"Added.")) ok_add=1;
        if(strstr(line,"Updated")) ok_upd=1;
        if(strstr(line,"Marked Inactive")) ok_del=1;
        if(strstr(line,"Bye!")) ok_exit=1;
    }
    fclose(f);

    printf("[E2E] Add %s\n",   ok_add ?"OK":"FAIL");
    printf("[E2E] Update %s\n",ok_upd ?"OK":"FAIL");
    printf("[E2E] Delete %s\n",ok_del ?"OK":"FAIL");
    printf("[E2E] Exit %s\n",  ok_exit?"OK":"FAIL");

    return (ok_add&&ok_upd&&ok_del&&ok_exit)?0:1;
}
