#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_ROWS   4096
#define MAX_STR    128
#define MAX_PHONE  64

typedef struct {
    char company[MAX_STR];
    char contact[MAX_STR];
    char phone[MAX_PHONE];
    char email[MAX_STR];
    char status[16];         /* "Active" | "Inactive" (soft delete) */
} Customer;

typedef struct {
    Customer items[MAX_ROWS];
    int count;
} CustomerDB;

/* ---------- Globals ---------- */
static CustomerDB db;
static char DATA_PATH[256] = "customers.csv";

/* ---------- Prototypes used by main/tests ---------- */
void open_file(void);
void list_users(void);
void list_inactive(void);
void add_user(void);
void search_user(void);
void edit_user(void);
void delete_user(void);     /* multi-select + confirm */
void restore_user(void);    /* multi-select + confirm */
void run_unit_test(void);
void run_e2e_test(void);
void set_data_path(const char *path);

/* ---------- Utils ---------- */
static void safe_copy(char *dst, const char *src, size_t cap){
    if(!dst || cap==0) return;
    if(!src){ dst[0]='\0'; return; }
    strncpy(dst, src, cap-1);
    dst[cap-1] = '\0';
}
static void rstrip(char *s){
    size_t n = strlen(s);
    while(n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]='\0';
}

/* ---------- Validation ---------- */
static bool valid_email(const char* s){
    if(!s || !*s) return false;
    if(strchr(s,' ')) return false;
    const char *at = strchr(s,'@');
    const char *dot = strrchr(s,'.');
    if(!at || !dot) return false;
    if(at==s) return false;
    if(dot<=at) return false;
    if(*(dot+1)=='\0') return false;
    return true;
}
static bool valid_phone(const char* s){
    if(!s) return false;
    size_t n = strlen(s);
    if(n<9 || n>15) return false;
    for(size_t i=0;i<n;i++) if(!isdigit((unsigned char)s[i])) return false;
    return true;
}

/* ---------- CSV I/O ---------- */
void set_data_path(const char *path){
    if(path) safe_copy(DATA_PATH, path, sizeof(DATA_PATH));
}
static void ensure_csv_exists(void){
    FILE *f = fopen(DATA_PATH, "r");
    if(f){ fclose(f); return; }
    f = fopen(DATA_PATH, "w");
    if(!f){ perror("create csv"); return; }
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email,Status\n");
    fclose(f);
}
static void load_csv(void){
    db.count=0;
    FILE *f = fopen(DATA_PATH, "r");
    if(!f){ perror("open csv"); return; }
    char line[512];
    if(!fgets(line,sizeof(line),f)){ fclose(f); return; } /* header */
    while(fgets(line,sizeof(line),f) && db.count<MAX_ROWS){
        rstrip(line);
        if(line[0]=='\0') continue;
        Customer c={{0}};
        char *tok=strtok(line,",");
        if(tok) safe_copy(c.company,tok,MAX_STR);
        tok=strtok(NULL,","); if(tok) safe_copy(c.contact,tok,MAX_STR);
        tok=strtok(NULL,","); if(tok) safe_copy(c.phone,tok,MAX_PHONE);
        tok=strtok(NULL,","); if(tok) safe_copy(c.email,tok,MAX_STR);
        tok=strtok(NULL,","); if(tok) safe_copy(c.status,tok,sizeof(c.status));
        else safe_copy(c.status,"Active",sizeof(c.status)); /* backward compatible */
        db.items[db.count++]=c;
    }
    fclose(f);
}
static void save_csv(void){
    FILE *f=fopen(DATA_PATH,"w");
    if(!f){ perror("save csv"); return; }
    fprintf(f,"CompanyName,ContactPerson,PhoneNumber,Email,Status\n");
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        fprintf(f,"%s,%s,%s,%s,%s\n", p->company,p->contact,p->phone,p->email,
                p->status[0]?p->status:"Active");
    }
    fclose(f);
}

/* ---------- UI Helpers ---------- */
static bool ask_confirm(const char *prompt){
    char ans[32];
    printf("%s [y/N]: ", prompt);
    if(!fgets(ans,sizeof(ans),stdin)) return false;
    rstrip(ans);
    return (ans[0]=='y' || ans[0]=='Y');
}
static int parse_index_list(const char *s, int max, int out_idx[], int out_cap){
    int cnt=0; char buf[256]; safe_copy(buf,s,sizeof(buf));
    char *tok=strtok(buf,", ");
    while(tok && cnt<out_cap){
        int k=atoi(tok);
        if(k>=1 && k<=max) out_idx[cnt++]=k-1;
        tok=strtok(NULL,", ");
    }
    return cnt;
}

/* ---------- Public API ---------- */
void open_file(void){ ensure_csv_exists(); load_csv(); }

void list_users(void){
    printf("CompanyName           | ContactPerson        | PhoneNumber      | Email\n");
    printf("-------------------------------------------------------------------------------\n");
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].status,"Inactive")==0) continue;
        printf("%-21s | %-20s | %-16s | %s\n",
               db.items[i].company, db.items[i].contact,
               db.items[i].phone, db.items[i].email);
    }
}
void list_inactive(void){
    printf("[Inactive only]\n");
    printf("CompanyName           | ContactPerson        | PhoneNumber      | Email\n");
    printf("-------------------------------------------------------------------------------\n");
    for(int i=0;i<db.count;i++){
        if(strcmp(db.items[i].status,"Inactive")==0){
            printf("%-21s | %-20s | %-16s | %s\n",
                   db.items[i].company, db.items[i].contact,
                   db.items[i].phone, db.items[i].email);
        }
    }
}

void add_user(void){
    Customer c={{0}}; char buf[256];
    puts("=== Add Customer (กรอกตามฟอร์ม) ===");
    puts("ตัวอย่าง:");
    puts("  CompanyName   : Rocket Co");
    puts("  ContactPerson : Rita Ray");
    puts("  PhoneNumber   : 0812345678   (ตัวเลข 9–15 หลัก)");
    puts("  Email         : rita@rocket.io (ต้องมี @ และมี . หลัง @)");
    puts("====================================");

    printf("CompanyName   : ");
    if(!fgets(buf,sizeof(buf),stdin)){ return; }
    rstrip(buf); safe_copy(c.company,buf,MAX_STR);

    printf("ContactPerson : ");
    if(!fgets(buf,sizeof(buf),stdin)){ return; }
    rstrip(buf); safe_copy(c.contact,buf,MAX_STR);

    for(;;){
        printf("PhoneNumber   : ");
        if(!fgets(buf,sizeof(buf),stdin)){ return; }
        rstrip(buf);
        if(valid_phone(buf)){ safe_copy(c.phone,buf,MAX_PHONE); break; }
        puts("  ✗ Invalid phone (ต้องเป็นตัวเลข 9–15 หลัก). ลองใหม่อีกครั้ง.");
    }
    for(;;){
        printf("Email         : ");
        if(!fgets(buf,sizeof(buf),stdin)){ return; }
        rstrip(buf);
        if(valid_email(buf)){ safe_copy(c.email,buf,MAX_STR); break; }
        puts("  ✗ Invalid email (ต้องมี @ และมี . หลัง @). ลองใหม่อีกครั้ง.");
    }
    safe_copy(c.status,"Active",sizeof(c.status));

    if(db.count<MAX_ROWS){
        db.items[db.count++]=c;
        save_csv();
        puts("✓ Added.");
    }else{
        puts("Database full.");
    }
}

void search_user(void){
    char key[128];
    printf("Search by company/contact/phone/email: ");
    if(!fgets(key,sizeof(key),stdin)) return;
    rstrip(key);
    int found=0;
    printf("CompanyName           | ContactPerson        | PhoneNumber      | Email\n");
    printf("-------------------------------------------------------------------------------\n");
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0) continue;
        if(strstr(p->company,key) || strstr(p->contact,key) ||
           strstr(p->phone,key)   || strstr(p->email,key)){
            printf("%-21s | %-20s | %-16s | %s\n",
                   p->company,p->contact,p->phone,p->email);
            found++;
        }
    }
    if(!found) puts("No records found.");
}

void edit_user(void){
    char ident[128], field[32], value[256];
    printf("Identifier to match: ");
    if(!fgets(ident,sizeof(ident),stdin)){ return; }
    rstrip(ident);

    printf("Field to update [CompanyName|ContactPerson|PhoneNumber|Email]: ");
    if(!fgets(field,sizeof(field),stdin)){ return; }
    rstrip(field);

    printf("New value: ");
    if(!fgets(value,sizeof(value),stdin)){ return; }
    rstrip(value);

    int updated=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0) continue;
        if(strstr(p->company,ident) || strstr(p->contact,ident) ||
           strstr(p->phone,ident)   || strstr(p->email,ident)){
            if(strcmp(field,"CompanyName")==0){ safe_copy(p->company,value,MAX_STR); }
            else if(strcmp(field,"ContactPerson")==0){ safe_copy(p->contact,value,MAX_STR); }
            else if(strcmp(field,"PhoneNumber")==0){
                if(!valid_phone(value)){ puts("Invalid phone."); continue; }
                safe_copy(p->phone,value,MAX_PHONE);
            }
            else if(strcmp(field,"Email")==0){
                if(!valid_email(value)){ puts("Invalid email."); continue; }
                safe_copy(p->email,value,MAX_STR);
            }
            else { puts("Unknown field."); continue; }
            updated++;
        }
    }
    if(updated){ save_csv(); printf("Updated %d record(s).\n", updated); }
    else puts("No records updated.");
}

/* ----- delete: multi-select + confirm ----- */
void delete_user(void){
    char ident[128];
    printf("Identifier to delete (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)){ return; }
    rstrip(ident);

    int idx[1024]; int n=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(strstr(p->company,ident) || strstr(p->contact,ident) ||
           strstr(p->phone,ident)   || strstr(p->email,ident)){
            idx[n++]=i;
            if(n==(int)(sizeof(idx)/sizeof(idx[0]))) break;
        }
    }
    if(n==0){ puts("No records found."); return; }

    if(n==1){
        if(!ask_confirm("Delete this record?")){ puts("Cancelled."); return; }
        safe_copy(db.items[idx[0]].status,"Inactive",sizeof(db.items[0].status));
        save_csv();
        puts("✓ Marked Inactive (1 record).");
        return;
    }

    printf("Found %d matches:\n", n);
    for(int j=0;j<n;j++){
        Customer *p=&db.items[idx[j]];
        printf("%d) %s | %s | %s | %s | %s\n", j+1,
               p->company,p->contact,p->phone,p->email,p->status);
    }
    printf("Delete all (A) or select indices (e.g., 1,3,5): ");
    char sel[256]; if(!fgets(sel,sizeof(sel),stdin)) return; rstrip(sel);

    int chosen[1024]; int cnum=0;
    bool del_all = (sel[0]=='A' || sel[0]=='a');

    if(!del_all){
        cnum = parse_index_list(sel,n,chosen,(int)(sizeof(chosen)/sizeof(chosen[0])));
        if(cnum<=0){ puts("No valid indices. Cancelled."); return; }
        puts("You will delete:");
        for(int t=0;t<cnum;t++){
            Customer *p=&db.items[idx[chosen[t]]];
            printf(" - %s | %s | %s | %s\n", p->company,p->contact,p->phone,p->email);
        }
        if(!ask_confirm("Confirm delete selected record(s)?")){ puts("Cancelled."); return; }
        for(int t=0;t<cnum;t++)
            safe_copy(db.items[idx[chosen[t]]].status,"Inactive",sizeof(db.items[0].status));
        save_csv();
        printf("✓ Marked Inactive (%d record(s)).\n", cnum);
    }else{
        if(!ask_confirm("Confirm delete ALL matched record(s)?")){ puts("Cancelled."); return; }
        for(int j=0;j<n;j++)
            safe_copy(db.items[idx[j]].status,"Inactive",sizeof(db.items[0].status));
        save_csv();
        printf("✓ Marked Inactive (%d record(s)).\n", n);
    }
}

/* ----- restore: multi-select + confirm ----- */
void restore_user(void){
    char ident[128];
    printf("Identifier to restore (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)){ return; }
    rstrip(ident);

    int idx[1024]; int n=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0 &&
           (strstr(p->company,ident) || strstr(p->contact,ident) ||
            strstr(p->phone,ident)   || strstr(p->email,ident))){
            idx[n++]=i;
        }
    }
    if(n==0){ puts("No inactive records found."); return; }

    if(n==1){
        if(!ask_confirm("Restore this record?")){ puts("Cancelled."); return; }
        safe_copy(db.items[idx[0]].status,"Active",sizeof(db.items[0].status));
        save_csv();
        puts("✓ Restored (1 record).");
        return;
    }

    printf("Found %d inactive matches:\n", n);
    for(int j=0;j<n;j++){
        Customer *p=&db.items[idx[j]];
        printf("%d) %s | %s | %s | %s | %s\n", j+1,
               p->company,p->contact,p->phone,p->email,p->status);
    }
    printf("Restore all (A) or select indices (e.g., 2,4): ");
    char sel[256]; if(!fgets(sel,sizeof(sel),stdin)) return; rstrip(sel);

    int chosen[1024]; int cnum=0;
    bool all = (sel[0]=='A' || sel[0]=='a');

    if(!all){
        cnum = parse_index_list(sel,n,chosen,(int)(sizeof(chosen)/sizeof(chosen[0])));
        if(cnum<=0){ puts("No valid indices. Cancelled."); return; }
        puts("You will restore:");
        for(int t=0;t<cnum;t++){
            Customer *p=&db.items[idx[chosen[t]]];
            printf(" - %s | %s | %s | %s\n", p->company,p->contact,p->phone,p->email);
        }
        if(!ask_confirm("Confirm restore selected record(s)?")){ puts("Cancelled."); return; }
        for(int t=0;t<cnum;t++)
            safe_copy(db.items[idx[chosen[t]]].status,"Active",sizeof(db.items[0].status));
        save_csv();
        printf("✓ Restored (%d record(s)).\n", cnum);
    }else{
        if(!ask_confirm("Confirm restore ALL matched record(s)?")){ puts("Cancelled."); return; }
        for(int j=0;j<n;j++)
            safe_copy(db.items[idx[j]].status,"Active",sizeof(db.items[0].status));
        save_csv();
        printf("✓ Restored (%d record(s)).\n", n);
    }
}

/* ---------- Test runners via system() ---------- */
void run_unit_test(void){
    printf("Running unit tests...\n");
    int rc = system("gcc -std=c11 -O2 -Wall -Wextra -pedantic tests/test_unit.c -o tests/test_unit && ./tests/test_unit");
    if(rc!=0) puts("Unit test failed to run. Check tests/test_unit.c and paths.");
}
void run_e2e_test(void){
    printf("Running E2E test...\n");
    int rc = system("./crm < tests/e2e_input.txt > tests/e2e_output.txt");
    if(rc!=0){ puts("E2E runner failed. Is ./crm built and tests/e2e_input.txt present?"); return; }
    system("grep -q \"Added\\.\"          tests/e2e_output.txt && echo \"[E2E] Add OK\"      || echo \"[E2E] Add FAIL\"");
    system("grep -q \"Updated\"           tests/e2e_output.txt && echo \"[E2E] Update OK\"   || echo \"[E2E] Update FAIL\"");
    system("grep -q \"Marked Inactive\"   tests/e2e_output.txt && echo \"[E2E] Delete OK\"   || echo \"[E2E] Delete FAIL\"");
    system("grep -q \"Bye!\"              tests/e2e_output.txt && echo \"[E2E] Exit OK\"     || echo \"[E2E] Exit FAIL\"");
}
