// customer_manager.c
// Build with: gcc main.c customer_manager.c -std=c11 -O2 -Wall -Wextra -pedantic -o crm

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_ROWS   4096
#define MAX_STR    128
#define MAX_PHONE  64

typedef struct {
    char company[MAX_STR];
    char contact[MAX_STR];
    char phone[MAX_PHONE];
    char email[MAX_STR];
    char status[16]; // "Active" | "Inactive"
} Customer;

typedef struct {
    Customer items[MAX_ROWS];
    int count;
} CustomerDB;

static CustomerDB db;
static char DATA_PATH[256] = "customers.csv";

/* ========== small utils ========== */
static void rstrip(char *s){ size_t n=strlen(s); while(n && (s[n-1]=='\n'||s[n-1]=='\r')) s[--n]='\0'; }
static void trim(char *s){ char *p=s; while(*p && isspace((unsigned char)*p)) p++; if(p!=s) memmove(s,p,strlen(p)+1);
                           for(int i=(int)strlen(s)-1;i>=0 && isspace((unsigned char)s[i]);i--) s[i]='\0'; }
static void tolower_inplace(char *s){ for(;*s;++s) *s=(char)tolower((unsigned char)*s); }
static void safe_copy(char *dst,const char*src,size_t cap){ if(cap==0) return; strncpy(dst,src,cap-1); dst[cap-1]='\0'; }

/* strstr แบบไม่สนพิมพ์เล็ก-ใหญ่ (คืน pointer ตำแหน่งจริงใน haystack ถ้าพบ) */
static const char* istrstr(const char* haystack, const char* needle){
    static char hbuf[MAX_STR*2], nbuf[MAX_STR*2];
    safe_copy(hbuf, haystack, sizeof(hbuf));
    safe_copy(nbuf, needle,   sizeof(nbuf));
    tolower_inplace(hbuf); tolower_inplace(nbuf);
    char* pos = strstr(hbuf, nbuf);
    if(!pos) return NULL;
    return haystack + (pos - hbuf);
}

/* ========== CSV helpers (escape/parse) ========== */
/* เขียนค่า field ให้ปลอดภัย: ถ้ามี comma/quote/newline จะใส่ "..." และ escape "" -> """" */
static void csv_escape(FILE* f, const char* s){
    bool need_quote=false;
    for(const char* p=s; *p; ++p){
        if(*p==',' || *p=='"' || *p=='\n' || *p=='\r'){ need_quote=true; break; }
    }
    if(!need_quote){ fputs(s,f); return; }
    fputc('"',f);
    for(const char* p=s; *p; ++p){
        if(*p=='"') fputc('"',f); /* duplicate quote */
        fputc(*p,f);
    }
    fputc('"',f);
}

/* parser แบบง่าย: รองรับ field ที่มี "..." และ quote ซ้อน "" */
static int csv_parse_line(const char* line, char out[][MAX_STR], int max_fields){
    int nf=0;
    const char* p=line;
    while(*p && nf<max_fields){
        char buf[MAX_STR]; int bi=0;
        if(*p=='"'){ /* quoted field */
            p++;
            while(*p){
                if(*p=='"' && *(p+1)=='"'){ if(bi<MAX_STR-1) buf[bi++]='"'; p+=2; continue; }
                if(*p=='"'){ p++; break; }
                if(bi<MAX_STR-1) buf[bi++]=*p;
                p++;
            }
            buf[bi]='\0';
            out[nf][0]='\0'; safe_copy(out[nf], buf, MAX_STR);
            nf++;
            if(*p==',') p++;
        }else{ /* plain */
            while(*p && *p!=',' && *p!='\n' && *p!='\r'){
                if(bi<MAX_STR-1) buf[bi++]=*p;
                p++;
            }
            buf[bi]='\0';
            out[nf][0]='\0'; safe_copy(out[nf], buf, MAX_STR);
            nf++;
            if(*p==',') p++;
        }
    }
    return nf;
}

/* ========== Validation & business rules ========== */
static bool valid_company(const char* s){
    int len=(int)strlen(s); if(len<2||len>80) return false;
    if(isspace((unsigned char)s[0])||isspace((unsigned char)s[len-1])) return false;
    int hasalpha=0, consec=1;
    for(int i=0;i<len;i++){
        unsigned char c=(unsigned char)s[i];
        if(isalpha(c)) hasalpha=1;
        if(!(isalnum(c) || isspace(c) || c=='.' || c=='-' || c=='/' || c=='&' || c=='(' || c==')' || c=='\'' || c==',' ))
            return false;
        if(i>0 && !isalnum(c) && !isspace(c)){
            if(!isalnum((unsigned char)s[i-1]) && !isspace((unsigned char)s[i-1])){
                consec++; if(consec>2) return false;
            }else consec=1;
        }else consec=1;
    }
    return hasalpha;
}
static bool valid_contact(const char* s){
    int len=(int)strlen(s); if(len<1||len>80) return false;
    int hasalpha=0;
    for(int i=0;i<len;i++){
        unsigned char c=(unsigned char)s[i];
        if(isalpha(c)) hasalpha=1;
        if(!(isalnum(c) || isspace(c) || c=='.' || c=='-' || c=='/' || c=='&' || c=='(' || c==')' || c=='\'' || c==',' ))
            return false;
    }
    return hasalpha;
}
static void normalize_phone(char* s){
    if(strncmp(s,"+66",3)==0){ char t[MAX_PHONE]; snprintf(t,sizeof(t),"0%s",s+3); safe_copy(s,t,MAX_PHONE); }
}
static bool valid_phone_strict(const char* s){
    if(s[0]=='\0') return true; /* optional */
    int len=(int)strlen(s); if(len<9||len>15) return false;
    if(s[0]!='0') return false;
    for(int i=0;i<len;i++) if(!isdigit((unsigned char)s[i])) return false;
    /* prefix check */
    if(!( (s[0]=='0') && (s[1]=='2'||s[1]=='3'||s[1]=='6'||s[1]=='8'||s[1]=='9') )) return false;
    return true;
}
static bool valid_email_strict(const char* s){
    if(s[0]=='\0') return true; /* optional */
    if(strchr(s,' ') || strstr(s,"..")) return false;
    const char* at=strchr(s,'@'); if(!at || strchr(at+1,'@')) return false;
    const char* dot=strrchr(at,'.'); if(!dot || dot<=at+1) return false;
    if(strlen(dot+1)<2) return false;
    return true;
}
static bool has_at_least_one_contact(Customer* c){ return (c->phone[0]!='\0' || c->email[0]!='\0'); }

/* กันข้อมูลซ้ำ: ถ้ามี email ใช้ (company,contact,email) / ถ้า email ว่างใช้ (company,contact,phone) */
static bool is_duplicate(const Customer* c, int skip_index){
    for(int i=0;i<db.count;i++){
        if(i==skip_index) continue;
        const Customer *p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0) continue;
        if(c->email[0]){
            if(strcmp(c->company,p->company)==0 && strcmp(c->contact,p->contact)==0 && strcmp(c->email,p->email)==0)
                return true;
        }else{
            if(strcmp(c->company,p->company)==0 && strcmp(c->contact,p->contact)==0 && strcmp(c->phone,p->phone)==0)
                return true;
        }
    }
    return false;
}

/* ========== IO: open/save ========== */
void set_data_path(const char* path){ safe_copy(DATA_PATH, path, sizeof(DATA_PATH)); }

static void ensure_csv_exists(void){
    FILE* f=fopen(DATA_PATH,"r");
    if(f){ fclose(f); return; }
    f=fopen(DATA_PATH,"w");
    if(!f){ perror("create csv"); return; }
    fprintf(f,"CompanyName,ContactPerson,PhoneNumber,Email,Status\n");
    fclose(f);
}

void open_file(void){
    ensure_csv_exists();
    db.count=0;
    FILE* f=fopen(DATA_PATH,"r");
    if(!f){ perror("open csv"); return; }
    char line[1024];
    if(!fgets(line,sizeof(line),f)){ fclose(f); return; } /* header */
    while(fgets(line,sizeof(line),f) && db.count<MAX_ROWS){
        rstrip(line);
        if(line[0]=='\0') continue;
        char fields[5][MAX_STR]={{0}};
        int n=csv_parse_line(line, fields, 5);
        if(n<1) continue;
        Customer c={{0}};
        if(n>=1) safe_copy(c.company, fields[0], MAX_STR);
        if(n>=2) safe_copy(c.contact, fields[1], MAX_STR);
        if(n>=3) safe_copy(c.phone,   fields[2], MAX_PHONE);
        if(n>=4) safe_copy(c.email,   fields[3], MAX_STR);
        if(n>=5) safe_copy(c.status,  fields[4], sizeof(c.status)); else safe_copy(c.status,"Active",sizeof(c.status));
        db.items[db.count++]=c;
    }
    fclose(f);
}

static void save_csv(void){
    FILE* f=fopen(DATA_PATH,"w");
    if(!f){ perror("save csv"); return; }
    fprintf(f,"CompanyName,ContactPerson,PhoneNumber,Email,Status\n");
    for(int i=0;i<db.count;i++){
        Customer* c=&db.items[i];
        csv_escape(f,c->company); fputc(',',f);
        csv_escape(f,c->contact); fputc(',',f);
        csv_escape(f,c->phone);   fputc(',',f);
        csv_escape(f,c->email);   fputc(',',f);
        csv_escape(f,c->status);  fputc('\n',f);
    }
    fclose(f);
}

/* ========== helpers for multi-select / confirm ========== */
static int parse_index_list(const char* s, int maxn, int *out, int outcap){
    int n=0; char tmp[256]; safe_copy(tmp,s,sizeof(tmp));
    char* tok=strtok(tmp,",");
    while(tok && n<outcap){
        int v=atoi(tok);
        if(v>=1 && v<=maxn) out[n++]=v-1;
        tok=strtok(NULL,",");
    }
    return n;
}
static bool ask_confirm(const char* msg){
    char a[8];
    printf("%s [y/N]: ", msg);
    if(!fgets(a,sizeof(a),stdin)) return false;
    rstrip(a); trim(a);
    return (a[0]=='y'||a[0]=='Y');
}

/* ========== printing ========== */
static void print_header(bool inactive){
    if(inactive) puts("[Inactive only]");
    printf("%-21s | %-20s | %-16s | %s\n","CompanyName","ContactPerson","PhoneNumber","Email");
    puts("-------------------------------------------------------------------------------");
}

/* ========== list/search ========== */
void list_users(void){
    print_header(false);
    for(int i=0;i<db.count;i++){
        Customer* c=&db.items[i];
        if(strcmp(c->status,"Inactive")==0) continue;
        printf("%-21s | %-20s | %-16s | %s\n",c->company,c->contact,c->phone,c->email);
    }
}
void list_inactive(void){
    print_header(true);
    for(int i=0;i<db.count;i++){
        Customer* c=&db.items[i];
        if(strcmp(c->status,"Inactive")==0)
            printf("%-21s | %-20s | %-16s | %s\n",c->company,c->contact,c->phone,c->email);
    }
}
void search_user(void){
    char key[128];
    printf("Search by company/contact/phone/email: ");
    if(!fgets(key,sizeof(key),stdin)) return; rstrip(key); trim(key);
    print_header(false);
    int found=0;
    for(int i=0;i<db.count;i++){
        Customer* c=&db.items[i];
        if(strcmp(c->status,"Inactive")==0) continue;
        if( istrstr(c->company,key) || istrstr(c->contact,key) ||
            istrstr(c->phone,key)   || istrstr(c->email,key) ){
            printf("%-21s | %-20s | %-16s | %s\n",c->company,c->contact,c->phone,c->email);
            found++;
        }
    }
    if(!found) puts("No records found.");
}

/* ========== add/update/delete/restore ========== */
void add_user(void){
    char company[MAX_STR], contact[MAX_STR], phone[MAX_PHONE], email[MAX_STR];
    puts("=== Add Customer (STRICT + Practical) ===");
    puts("กติกา:");
    puts(" • Company/Contact: 2–80 ตัว, ต้องมี 'ตัวอักษร' อย่างน้อย 1, อนุญาต . - / & ( ) ' , และเว้นวรรค; ห้ามขึ้นต้น/ลงท้ายด้วยเครื่องหมาย");
    puts(" • Phone: ตัวเลขล้วน 9–15 หลัก, เริ่มด้วย 0, prefix 02/03/06/08/09 (รองรับ +66 -> 0)");
    puts(" • Email: มี @ เดียว, โดเมนมีจุด, ไม่มีช่องว่าง/.. , TLD >= 2 ตัวอักษร");
    puts(" • ต้องมีอย่างน้อย 1 ช่องทางติดต่อ (Phone หรือ Email)");
    puts("----------------------------------------");

    printf("CompanyName   : "); if(!fgets(company,sizeof(company),stdin)) return; rstrip(company); trim(company);
    if(!valid_company(company)){ puts("× Company ไม่ถูกต้อง: ต้องยาว 2–80 ตัว, มีตัวอักษรอย่างน้อย 1, ห้ามขึ้นต้น/ลงท้ายด้วยเครื่องหมาย และไม่มีอักขระผิดกติกา"); return; }

    printf("ContactPerson : "); if(!fgets(contact,sizeof(contact),stdin)) return; rstrip(contact); trim(contact);
    if(!valid_contact(contact)){ puts("× Contact ไม่ถูกต้อง: กติกาเดียวกับ Company"); return; }

    printf("PhoneNumber   (enter to skip): "); if(!fgets(phone,sizeof(phone),stdin)) return; rstrip(phone); trim(phone);
    if(phone[0]){ normalize_phone(phone); if(!valid_phone_strict(phone)){ puts("× Invalid phone (ตัวเลข 9–15, เริ่ม 0, prefix 02/03/06/08/09)."); return; } }

    printf("Email         (enter to skip): "); if(!fgets(email,sizeof(email),stdin)) return; rstrip(email); trim(email);
    if(!valid_email_strict(email)){ puts("× Invalid email."); return; }

    Customer c={{0}};
    safe_copy(c.company,company,MAX_STR);
    safe_copy(c.contact,contact,MAX_STR);
    safe_copy(c.phone,phone,MAX_PHONE);
    safe_copy(c.email,email,MAX_STR);
    safe_copy(c.status,"Active",sizeof(c.status));

    if(!has_at_least_one_contact(&c)){ puts("× ต้องมีอย่างน้อย 1 ช่องทางติดต่อ (Phone หรือ Email)"); return; }
    if(is_duplicate(&c,-1)){ puts("× Duplicate: ข้อมูลนี้มีอยู่แล้ว"); return; }

    if(db.count<MAX_ROWS){ db.items[db.count++]=c; save_csv(); puts("✓ Added."); }
    else puts("Database full.");
}

void edit_user(void){
    char ident[128];
    printf("Identifier to match (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident); trim(ident);

    int idx[1024], n=0;
    for(int i=0;i<db.count;i++){
        Customer* p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0) continue;
        if( istrstr(p->company,ident) || istrstr(p->contact,ident) ||
            istrstr(p->phone,ident)   || istrstr(p->email,ident) ){
            idx[n++]=i;
            if(n==(int)(sizeof(idx)/sizeof(idx[0]))) break;
        }
    }
    if(n==0){ puts("No active records found."); return; }
    if(n==1){
        Customer *p=&db.items[idx[0]];
        printf("Matched 1 record:\n1) %s | %s | %s | %s\n",p->company,p->contact,p->phone,p->email);
    }else{
        printf("Found %d matches:\n",n);
        for(int j=0;j<n;j++){
            Customer* p=&db.items[idx[j]];
            printf("%d) %s | %s | %s | %s\n",j+1,p->company,p->contact,p->phone,p->email);
        }
    }

    char select[256];
    if(n>1){ printf("Update all (A) or select indices (e.g., 1,3,5): ");
             if(!fgets(select,sizeof(select),stdin)) return; rstrip(select); trim(select); }
    else strcpy(select,"A");

    int chosen[1024], cnum=0;
    bool all = (select[0]=='A'||select[0]=='a');
    if(!all){
        cnum = parse_index_list(select,n,chosen,(int)(sizeof(chosen)/sizeof(chosen[0])));
        if(cnum<=0){ puts("No valid indices. Cancelled."); return; }
    }else{ for(int j=0;j<n;j++) chosen[cnum++]=j; }

    char field[32], value[256];
    printf("Field to update [CompanyName|ContactPerson|PhoneNumber|Email]: ");
    if(!fgets(field,sizeof(field),stdin)) return; rstrip(field); trim(field);

    printf("New value (empty allowed only for Phone/Email): ");
    if(!fgets(value,sizeof(value),stdin)) return; rstrip(value); trim(value);

    puts("Preview changes:");
    for(int t=0;t<cnum;t++){
        Customer* p=&db.items[idx[chosen[t]]];
        printf(" - %s | %s | %s | %s  =>  ",p->company,p->contact,p->phone,p->email);
        if(strcmp(field,"CompanyName")==0)       printf("[%s -> %s]\n",p->company,value);
        else if(strcmp(field,"ContactPerson")==0)printf("[%s -> %s]\n",p->contact,value);
        else if(strcmp(field,"PhoneNumber")==0)  printf("[%s -> %s]\n",p->phone,value);
        else if(strcmp(field,"Email")==0)        printf("[%s -> %s]\n",p->email,value);
        else { puts("Unknown field."); return; }
    }
    if(!ask_confirm("Apply these changes?")){ puts("Cancelled."); return; }

    int updated=0, rejected=0;
    for(int t=0;t<cnum;t++){
        int i = idx[chosen[t]];
        Customer *p=&db.items[i], tmp=*p;

        if(strcmp(field,"CompanyName")==0){
            if(!valid_company(value)){ puts("Invalid company."); rejected++; continue; }
            safe_copy(tmp.company,value,MAX_STR);

        }else if(strcmp(field,"ContactPerson")==0){
            if(!valid_contact(value)){ puts("Invalid contact."); rejected++; continue; }
            safe_copy(tmp.contact,value,MAX_STR);

        }else if(strcmp(field,"PhoneNumber")==0){
            if(value[0]=='\0'){ tmp.phone[0]='\0'; }
            else { char ph[MAX_PHONE]; safe_copy(ph,value,MAX_PHONE); normalize_phone(ph);
                   if(!valid_phone_strict(ph)){ puts("Invalid phone."); rejected++; continue; }
                   safe_copy(tmp.phone,ph,MAX_PHONE); }

        }else if(strcmp(field,"Email")==0){
            if(value[0]=='\0'){ tmp.email[0]='\0'; }
            else { if(!valid_email_strict(value)){ puts("Invalid email."); rejected++; continue; }
                   safe_copy(tmp.email,value,MAX_STR); }
        }else { puts("Unknown field."); return; }

        if(!has_at_least_one_contact(&tmp)){ puts("Need at least one of Phone/Email."); rejected++; continue; }
        if(is_duplicate(&tmp,i)){ puts("Duplicate after update."); rejected++; continue; }

        *p = tmp; updated++;
    }

    if(updated){ save_csv(); printf("Updated %d record(s).\n", updated); }
    else if(rejected){ puts("No records updated (validation/duplicate failed)."); }
    else puts("No records updated.");
}

void delete_user(void){
    char ident[128];
    printf("Identifier to delete (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident); trim(ident);

    int idx[1024], n=0;
    for(int i=0;i<db.count;i++){
        Customer* p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0) continue;
        if( istrstr(p->company,ident) || istrstr(p->contact,ident) ||
            istrstr(p->phone,ident)   || istrstr(p->email,ident) ){
            idx[n++]=i; if(n==(int)(sizeof(idx)/sizeof(idx[0]))) break;
        }
    }
    if(n==0){ puts("No active records found."); return; }

    printf("Found %d matches:\n",n);
    for(int j=0;j<n;j++){
        Customer* p=&db.items[idx[j]];
        printf("%d) %s | %s | %s | %s | %s\n",j+1,p->company,p->contact,p->phone,p->email,p->status);
    }

    char sel[256];
    printf("Delete all (A) or select indices (e.g., 1,3,5): ");
    if(!fgets(sel,sizeof(sel),stdin)) return; rstrip(sel); trim(sel);

    int chosen[1024], cnum=0;
    bool all=(sel[0]=='A'||sel[0]=='a');
    if(!all){
        cnum=parse_index_list(sel,n,chosen,(int)(sizeof(chosen)/sizeof(chosen[0])));
        if(cnum<=0){ puts("No valid indices. Cancelled."); return; }
    }else for(int j=0;j<n;j++) chosen[cnum++]=j;

    puts("You will delete:");
    for(int t=0;t<cnum;t++){
        Customer* p=&db.items[idx[chosen[t]]];
        printf(" - %s | %s | %s | %s\n",p->company,p->contact,p->phone,p->email);
    }
    if(!ask_confirm(all?"Confirm delete ALL matched record(s)?":"Confirm delete selected record(s)?")){ puts("Cancelled."); return; }

    int del=0;
    for(int t=0;t<cnum;t++){
        Customer* p=&db.items[idx[chosen[t]]];
        if(strcmp(p->status,"Inactive")!=0){ safe_copy(p->status,"Inactive",sizeof(p->status)); del++; }
    }
    if(del){ save_csv(); printf("✓ Marked Inactive (%d record(s)).\n",del); }
    else puts("No records deleted.");
}

void restore_user(void){
    char ident[128];
    printf("Identifier to restore (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident); trim(ident);

    int idx[1024], n=0;
    for(int i=0;i<db.count;i++){
        Customer* p=&db.items[i];
        if(strcmp(p->status,"Inactive")!=0) continue;
        if( istrstr(p->company,ident) || istrstr(p->contact,ident) ||
            istrstr(p->phone,ident)   || istrstr(p->email,ident) ){
            idx[n++]=i; if(n==(int)(sizeof(idx)/sizeof(idx[0]))) break;
        }
    }
    if(n==0){ puts("No inactive records found."); return; }

    printf("Found %d inactive matches:\n",n);
    for(int j=0;j<n;j++){
        Customer* p=&db.items[idx[j]];
        printf("%d) %s | %s | %s | %s | %s\n",j+1,p->company,p->contact,p->phone,p->email,p->status);
    }

    char sel[256];
    printf("Restore all (A) or select indices (e.g., 2,4): ");
    if(!fgets(sel,sizeof(sel),stdin)) return; rstrip(sel); trim(sel);

    int chosen[1024], cnum=0;
    bool all=(sel[0]=='A'||sel[0]=='a');
    if(!all){
        cnum=parse_index_list(sel,n,chosen,(int)(sizeof(chosen)/sizeof(chosen[0])));
        if(cnum<=0){ puts("No valid indices. Cancelled."); return; }
    }else for(int j=0;j<n;j++) chosen[cnum++]=j;

    puts("You will restore:");
    for(int t=0;t<cnum;t++){
        Customer* p=&db.items[idx[chosen[t]]];
        printf(" - %s | %s | %s | %s\n",p->company,p->contact,p->phone,p->email);
    }
    if(!ask_confirm(all?"Confirm restore ALL matched record(s)?":"Confirm restore selected record(s)?")){ puts("Cancelled."); return; }

    int res=0;
    for(int t=0;t<cnum;t++){
        Customer* p=&db.items[idx[chosen[t]]];
        if(strcmp(p->status,"Inactive")==0){ safe_copy(p->status,"Active",sizeof(p->status)); res++; }
    }
    if(res){ save_csv(); printf("✓ Restored (%d record(s)).\n",res); }
    else puts("No records restored.");
}

/* ========== test runners (via system) ========== */
void run_unit_test(void){
    puts("Running unit tests...");
    int rc = system("gcc -std=c11 -O2 -Wall -Wextra -pedantic tests/test_unit.c -o tests/test_unit && ./tests/test_unit");
    if(rc!=0) puts("Unit test failed to run. Check tests/test_unit.c and paths.");
}
void run_e2e_test(void){
    puts("Running E2E test...");
    int rc = system("./crm < tests/e2e_input.txt > tests/e2e_output.txt && "
                    "grep -q \"Added.\" tests/e2e_output.txt && echo \"[E2E] Add OK\" || echo \"[E2E] Add FAIL\"; "
                    "grep -q \"Updated\" tests/e2e_output.txt && echo \"[E2E] Update OK\" || echo \"[E2E] Update FAIL\"; "
                    "grep -q \"Marked Inactive\" tests/e2e_output.txt && echo \"[E2E] Delete OK\" || echo \"[E2E] Delete FAIL\"; "
                    "grep -q \"Bye!\" tests/e2e_output.txt && echo \"[E2E] Exit OK\" || echo \"[E2E] Exit FAIL\"");
    (void)rc;
}

/* ========== menu ========== */
void display_menu(void){
    char line[32];
    for(;;){
        puts("==== Customer CRM ===");
        puts("1) List all");
        puts("2) Add");
        puts("3) Search");
        puts("4) Update field");
        puts("5) Delete (soft)");
        puts("6) Exit");
        puts("7) List inactive");
        puts("8) Restore");
        puts("9) Run Unit Test");
        puts("10) Run E2E Test");
        printf("Choose [1-10]: ");
        if(!fgets(line,sizeof(line),stdin)) return;
        int choice=atoi(line);
        if(choice==1) list_users();
        else if(choice==2) add_user();
        else if(choice==3) search_user();
        else if(choice==4) edit_user();
        else if(choice==5) delete_user();
        else if(choice==6){ puts("Bye!"); break; }
        else if(choice==7) list_inactive();
        else if(choice==8) restore_user();
        else if(choice==9) run_unit_test();
        else if(choice==10) run_e2e_test();
        else puts("Invalid choice.");
    }
}
