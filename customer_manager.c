#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_ROWS 4096
#define MAX_STR  128
#define MAX_PHONE 64

typedef struct {
    char company[MAX_STR];
    char contact[MAX_STR];
    char phone[MAX_PHONE];
    char email[MAX_STR];
    bool active;   // ใช้ทำ soft delete
} Customer;

typedef struct {
    Customer items[MAX_ROWS];
    int count;
} CustomerDB;

static CustomerDB db;
static char DATA_PATH[256] = "customers.csv";

/* ===== utils ===== */
static void rstrip(char *s){
    size_t n = strlen(s);
    while(n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]='\0';
}
static bool icontains(const char* hay, const char* needle){
    return strstr(hay, needle) != NULL;
}

/* ===== Validation ===== */
static bool valid_email(const char* s){
    return strchr(s,'@') && strchr(s,'.');
}
static bool valid_phone(const char* s){
    int len = strlen(s);
    if(len < 9 || len > 15) return false;
    for(int i=0;i<len;i++) if(!isdigit((unsigned char)s[i])) return false;
    return true;
}

/* ===== CSV I/O ===== */
static void ensure_csv_exists(void){
    FILE *f = fopen(DATA_PATH, "r");
    if (f){ fclose(f); return; }
    f = fopen(DATA_PATH, "w");
    if (!f) { perror("create csv"); return; }
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email,Active\n");
    fclose(f);
}
static void load_csv(void){
    db.count = 0;
    FILE *f = fopen(DATA_PATH, "r");
    if (!f){ perror("open csv"); return; }
    char line[512];
    if (!fgets(line, sizeof(line), f)){ fclose(f); return; }
    while (fgets(line, sizeof(line), f) && db.count < MAX_ROWS){
        rstrip(line);
        if(line[0]=='\0') continue;
        char *tok = strtok(line, ",");
        Customer c = {{0}};
        if(tok) strncpy(c.company, tok, MAX_STR-1);
        tok = strtok(NULL, ","); if(tok) strncpy(c.contact, tok, MAX_STR-1);
        tok = strtok(NULL, ","); if(tok) strncpy(c.phone, tok, MAX_PHONE-1);
        tok = strtok(NULL, ","); if(tok) strncpy(c.email, tok, MAX_STR-1);
        tok = strtok(NULL, ","); c.active = (!tok || strcmp(tok,"0")!=0);
        db.items[db.count++] = c;
    }
    fclose(f);
}
static void save_csv(void){
    FILE *f = fopen(DATA_PATH, "w");
    if (!f){ perror("save csv"); return; }
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email,Active\n");
    for (int i=0;i<db.count;i++){
        Customer *c=&db.items[i];
        fprintf(f, "%s,%s,%s,%s,%d\n", c->company,c->contact,c->phone,c->email,c->active?1:0);
    }
    fclose(f);
}

/* ===== API ===== */
void open_file(void){ ensure_csv_exists(); load_csv(); }
void list_users(void){
    printf("CompanyName           | ContactPerson        | PhoneNumber      | Email\n");
    printf("-------------------------------------------------------------------------------\n");
    for(int i=0;i<db.count;i++){
        if(!db.items[i].active) continue;
        printf("%-21s | %-20s | %-16s | %s\n",
            db.items[i].company, db.items[i].contact, db.items[i].phone, db.items[i].email);
    }
}
void list_inactive(void){
    printf("Inactive Records:\n");
    for(int i=0;i<db.count;i++){
        if(db.items[i].active) continue;
        printf("[%d] %-21s | %-20s | %-16s | %s\n", i+1,
            db.items[i].company, db.items[i].contact, db.items[i].phone, db.items[i].email);
    }
}

void add_user(void){
    Customer c = {{0}};
    char buf[256];
    puts("=== Add Customer (กรอกให้ตรงตามฟอร์ม) ===");
    puts("ตัวอย่าง:");
    puts("  CompanyName : Rocket Co");
    puts("  ContactPerson : Rita Ray");
    puts("  PhoneNumber : 0812345678 (ตัวเลขเท่านั้น 9–15 หลัก)");
    puts("  Email : rita@rocket.io (ต้องมี @ และ . หลัง @)");
    puts("===========================================");
    printf("CompanyName : "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.company, buf, MAX_STR-1);
    printf("ContactPerson: "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.contact, buf, MAX_STR-1);
    printf("PhoneNumber : "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.phone, buf, MAX_PHONE-1);
    printf("Email       : "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.email, buf, MAX_STR-1);

    if(!valid_phone(c.phone)){ puts("✗ Invalid phone."); return; }
    if(!valid_email(c.email)){ puts("✗ Invalid email."); return; }

    c.active = true;
    db.items[db.count++] = c;
    save_csv();
    puts("✓ Added.");
}

void delete_user(void){
    char ident[128];
    printf("Identifier to delete (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident);

    int idx[128], found=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(!p->active) continue;
        if(icontains(p->company,ident)||icontains(p->contact,ident)||
           icontains(p->phone,ident)||icontains(p->email,ident)){
            printf("[%d] %-21s | %-20s | %-16s | %s\n", i+1,p->company,p->contact,p->phone,p->email);
            idx[found++]=i;
        }
    }
    if(!found){ puts("No records found."); return; }

    printf("Enter index (e.g. 1,3 or A for all): ");
    char line[128]; if(!fgets(line,sizeof(line),stdin)) return; rstrip(line);

    int toDelete[128], delCount=0;
    if(strcmp(line,"A")==0||strcmp(line,"a")==0){
        for(int j=0;j<found;j++) toDelete[delCount++]=idx[j];
    } else {
        char *tok=strtok(line,",");
        while(tok){ int n=atoi(tok); if(n>0 && n<=db.count) toDelete[delCount++]=n-1; tok=strtok(NULL,","); }
    }
    if(!delCount){ puts("No valid index."); return; }

    printf("Are you sure to delete %d record(s)? (y/n): ",delCount);
    if(!fgets(line,sizeof(line),stdin)) return; rstrip(line);
    if(strcmp(line,"y")!=0 && strcmp(line,"Y")!=0){ puts("Cancelled."); return; }

    for(int j=0;j<delCount;j++) db.items[toDelete[j]].active=false;
    save_csv();
    printf("✓ Marked %d record(s) inactive.\n",delCount);
}

void restore_user(void){
    char ident[128];
    printf("Identifier to restore: ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident);
    int restored=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(!p->active && (icontains(p->company,ident)||icontains(p->contact,ident)||
                          icontains(p->phone,ident)||icontains(p->email,ident))){
            p->active=true; restored++;
        }
    }
    if(restored){ save_csv(); printf("Restored %d record(s).\n",restored); }
    else puts("No records restored.");
}

/* ===== Menu ===== */
void display_menu(void){
    int choice=0; char line[32];
    for(;;){
        puts("==== Customer CRM ===");
        puts("1) List all");
        puts("2) Add");
        puts("3) Search (not yet)");
        puts("4) Update field (not yet)");
        puts("5) Delete (soft)");
        puts("6) Exit");
        puts("7) List inactive");
        puts("8) Restore");
        puts("9) Run Unit Test");
        puts("10) Run E2E Test");
        printf("Choose [1-10]: ");
        if(!fgets(line,sizeof(line),stdin)) return;
        choice=atoi(line);
        if(choice==1) list_users();
        else if(choice==2) add_user();
        else if(choice==5) delete_user();
        else if(choice==6){ puts("Bye!"); break; }
        else if(choice==7) list_inactive();
        else if(choice==8) restore_user();
        else if(choice==9) system("gcc tests/test_unit.c -o tests/test_unit && ./tests/test_unit");
        else if(choice==10) system("./crm < tests/e2e_input.txt > tests/e2e_output.txt && grep \"E2E\" tests/e2e_output.txt || true");
        else puts("Invalid choice.");
    }
}
