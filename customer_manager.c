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
    char phone[MAX_PHONE];  /* digits only after normalize; may be "" */
    char email[MAX_STR];    /* may be "" */
    char status[16];        /* "Active" | "Inactive" (soft delete) */
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
static void trim(char *s){
    size_t n = strlen(s);
    size_t i=0; while(i<n && isspace((unsigned char)s[i])) i++;
    size_t j=n; while(j>i && isspace((unsigned char)s[j-1])) j--;
    if(i>0 || j<n){ memmove(s, s+i, j-i); s[j-i]='\0'; }
}

/* keep digits; if original started with +66, convert to 0xxxxxxxxx */
static void normalize_phone(char *s){
    if(!s) return;
    char orig[64]; safe_copy(orig, s, sizeof(orig));
    char digits[64]; int j=0;
    for(int i=0; orig[i] && j<63; ++i) if(isdigit((unsigned char)orig[i])) digits[j++]=orig[i];
    digits[j]='\0';

    if(strncmp(orig, "+66", 3)==0){
        /* remove non-digits after +66 */
        char after[64]; int k=0;
        for(int i=3; orig[i] && k<63; ++i) if(isdigit((unsigned char)orig[i])) after[k++]=orig[i];
        after[k]='\0';
        snprintf(s, 64, "0%s", after);
    }else{
        safe_copy(s, digits, 64);
    }
}

/* ---------- Validation (STRICT+Practical) ---------- */

/* Printable, no control chars. Allow comma/quote (we will CSV-quote when saving). */
static bool valid_company(const char *s){
    if(!s) return false;
    size_t n = strlen(s);
    if(n < 2 || n > 80) return false;
    for(size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)s[i];
        if(!isprint(c)) return false; /* no control chars */
    }
    return true;
}
static bool valid_contact(const char *s){
    if(!s) return false;
    size_t n = strlen(s);
    if(n < 2 || n > 80) return false;
    for(size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)s[i];
        if(!isprint(c)) return false;
    }
    return true;
}
/* 9–15 digits; must start with 0; Thai-ish prefixes 02/03/06/08/09 */
static bool valid_phone_strict(const char *s){
    if(!s) return false;
    size_t n = strlen(s);
    if(n < 9 || n > 15) return false;
    for(size_t i=0;i<n;i++) if(!isdigit((unsigned char)s[i])) return false;
    if(s[0] != '0') return false;
    if(!( (s[1]=='2') || (s[1]=='3') || (s[1]=='6') || (s[1]=='8') || (s[1]=='9') )) return false;
    return true;
}
/* Email strict-ish per practical rules */
static bool valid_email_strict(const char* s){
    if(!s || !*s) return false;
    if(strchr(s,' ')) return false;
    const char *at = strchr(s,'@');
    if(!at) return false;
    if(strchr(at+1,'@')) return false;
    const char *local = s;
    const char *domain = at+1;
    if(local==at) return false;
    if(*domain=='\0') return false;

    if(*local=='.' || *(at-1)=='.') return false;
    for(const char *p=local; p<at; ++p){
        unsigned char c=(unsigned char)*p;
        if(!(isalnum(c) || c=='.' || c=='_' || c=='%' || c=='+' || c=='-')) return false;
    }
    if(strstr(domain,"..")) return false;
    const char *lastdot = strrchr(domain,'.');
    if(!lastdot || lastdot==domain) return false;
    const char *p = domain;
    while(*p){
        const char *label = p; int len=0;
        while(*p && *p!='.'){
            unsigned char c=(unsigned char)*p;
            if(!(isalnum(c) || c=='-')) return false;
            p++; len++;
        }
        if(len==0) return false;
        if(label[0]=='-' || label[len-1]=='-') return false;
        if(*p=='.') p++;
    }
    if(!lastdot[1] || !lastdot[2]) return false;
    for(const char *q=lastdot+1; *q; ++q){
        if(!isalpha((unsigned char)*q)) return false;
    }
    return true;
}

/* At least one contact method (phone or email) must be present & valid */
static bool has_at_least_one_contact(const Customer *c){
    bool hasPhone = c->phone[0] != '\0';
    bool hasEmail = c->email[0] != '\0';
    return hasPhone || hasEmail;
}

/* Duplicate rule:
   - If email present: (Company, Contact, Email) must be unique.
   - Else:            (Company, Contact, Phone) must be unique. */
static bool is_duplicate(const Customer *c, int skip_index){
    for(int i=0;i<db.count;i++){
        if(i==skip_index) continue;
        const Customer *p=&db.items[i];
        /* compare case-insensitive for company/contact; exact for phone; case-insensitive for email */
        bool sameCC = (strcasecmp(p->company,c->company)==0 && strcasecmp(p->contact,c->contact)==0);
        if(!sameCC) continue;

        if(c->email[0]){
            if(p->email[0] && strcasecmp(p->email,c->email)==0) return true;
        }else{
            if(!p->email[0] && strcmp(p->phone,c->phone)==0) return true;
        }
    }
    return false;
}

/* ---------- CSV Quoting / Parsing ---------- */
/* Need quote if contains comma, quote, or newline */
static bool csv_needs_quote(const char *s){
    for(const char *p=s; *p; ++p){
        if(*p==',' || *p=='"' || *p=='\n' || *p=='\r') return true;
    }
    return false;
}
/* Write one field with CSV quoting to stream */
static void csv_write_field(FILE *f, const char *s){
    if(!s){ fputs("", f); return; }
    if(!csv_needs_quote(s)){ fputs(s, f); return; }
    fputc('"', f);
    for(const char *p=s; *p; ++p){
        if(*p=='"') fputc('"', f); /* escape by doubling */
        fputc(*p, f);
    }
    fputc('"', f);
}
/* Parse one CSV line with quotes (up to 5 fields). Return number of fields. */
static int csv_parse_line(const char *line, char out[][MAX_STR], int max_fields){
    int nf=0;
    const char *p=line;
    while(*p && nf<max_fields){
        char buf[MAX_STR]; int j=0;
        if(*p=='"'){ /* quoted */
            p++;
            while(*p && j<MAX_STR-1){
                if(*p=='"'){
                    if(*(p+1)=='"'){ buf[j++]='"'; p+=2; } /* escaped quote */
                    else { p++; break; } /* end quote */
                }else{
                    buf[j++]=*p++;
                }
            }
            buf[j]='\0';
            out[nf][0]='\0'; safe_copy(out[nf], buf, MAX_STR);
            if(*p==',') p++;
        }else{
            while(*p && *p!=',' && *p!='\n' && *p!='\r' && j<MAX_STR-1) buf[j++]=*p++;
            buf[j]='\0';
            out[nf][0]='\0'; safe_copy(out[nf], buf, MAX_STR);
            if(*p==',') p++;
        }
        nf++;
    }
    /* fill remaining as empty */
    for(int i=nf;i<max_fields;i++){ out[i][0]='\0'; }
    return nf;
}

/* ---------- CSV I/O ---------- */
void set_data_path(const char *path){ if(path) safe_copy(DATA_PATH, path, sizeof(DATA_PATH)); }

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
    char line[1024];
    if(!fgets(line,sizeof(line),f)){ fclose(f); return; } /* header */
    while(fgets(line,sizeof(line),f) && db.count<MAX_ROWS){
        rstrip(line);
        if(line[0]=='\0') continue;
        char fields[5][MAX_STR];
        csv_parse_line(line, fields, 5);
        Customer c={{0}};
        safe_copy(c.company, fields[0], MAX_STR);
        safe_copy(c.contact, fields[1], MAX_STR);
        safe_copy(c.phone,   fields[2], MAX_PHONE);
        safe_copy(c.email,   fields[3], MAX_STR);
        if(fields[4][0]) safe_copy(c.status, fields[4], sizeof(c.status));
        else safe_copy(c.status,"Active",sizeof(c.status));
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
        csv_write_field(f, p->company); fputc(',',f);
        csv_write_field(f, p->contact); fputc(',',f);
        csv_write_field(f, p->phone  ); fputc(',',f);
        csv_write_field(f, p->email  ); fputc(',',f);
        csv_write_field(f, p->status ); fputc('\n',f);
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
    puts("=== Add Customer (STRICT+Practical) ===");
    puts("ตัวอย่าง:");
    puts("  CompanyName   : ACME Co., Ltd.");
    puts("  ContactPerson : Rita Ray");
    puts("  PhoneNumber   : 0812345678   (ตัวเลข 9–15; รองรับ +66 -> 0; ว่างได้หากมี Email)");
    puts("  Email         : rita@acme.co.th (1 @, โดเมนมีจุด, TLD >= 2; ว่างได้หากมี Phone)");
    puts("=======================================");

    /* CompanyName */
    for(;;){
        printf("CompanyName   : ");
        if(!fgets(buf,sizeof(buf),stdin)) return;
        rstrip(buf); trim(buf);
        if(valid_company(buf)){ safe_copy(c.company,buf,MAX_STR); break; }
        puts("  ✗ Invalid company name (2–80 printable characters). ลองใหม่.");
    }
    /* ContactPerson */
    for(;;){
        printf("ContactPerson : ");
        if(!fgets(buf,sizeof(buf),stdin)) return;
        rstrip(buf); trim(buf);
        if(valid_contact(buf)){ safe_copy(c.contact,buf,MAX_STR); break; }
        puts("  ✗ Invalid contact (2–80 printable characters). ลองใหม่.");
    }
    /* PhoneNumber (optional) */
    for(;;){
        printf("PhoneNumber   (enter to skip): ");
        if(!fgets(buf,sizeof(buf),stdin)) return;
        rstrip(buf); trim(buf);
        if(buf[0]=='\0'){ c.phone[0]='\0'; break; }
        normalize_phone(buf);
        if(valid_phone_strict(buf)){ safe_copy(c.phone,buf,MAX_PHONE); break; }
        puts("  ✗ Invalid phone (ตัวเลข 9–15 หลัก, เริ่ม 0, prefix 02/03/06/08/09). ลองใหม่.");
    }
    /* Email (optional) */
    for(;;){
        printf("Email         (enter to skip): ");
        if(!fgets(buf,sizeof(buf),stdin)) return;
        rstrip(buf); trim(buf);
        if(buf[0]=='\0'){ c.email[0]='\0'; break; }
        if(valid_email_strict(buf)){ safe_copy(c.email,buf,MAX_STR); break; }
        puts("  ✗ Invalid email (1 @; โดเมนมีจุด; ไม่มีช่องว่าง/..; TLD >=2). ลองใหม่.");
    }
    /* At least one */
    if(!has_at_least_one_contact(&c)){
        puts("✗ Need at least one contact method (Phone or Email). Not added.");
        return;
    }
    /* Duplicate check */
    if(is_duplicate(&c, -1)){
        puts("✗ Duplicate record (unique by (Company,Contact,Email) or (Company,Contact,Phone)). Not added.");
        return;
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
    rstrip(key); trim(key);
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
    rstrip(ident); trim(ident);

    printf("Field to update [CompanyName|ContactPerson|PhoneNumber|Email]: ");
    if(!fgets(field,sizeof(field),stdin)){ return; }
    rstrip(field); trim(field);

    printf("New value (empty allowed only for Phone/Email): ");
    if(!fgets(value,sizeof(value),stdin)){ return; }
    rstrip(value); trim(value);

    int updated=0, rejected=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if(strcmp(p->status,"Inactive")==0) continue;
        if(strstr(p->company,ident) || strstr(p->contact,ident) ||
           strstr(p->phone,ident)   || strstr(p->email,ident)){

            Customer tmp = *p; /* simulate change then validate/duplicate */
            if(strcmp(field,"CompanyName")==0){
                if(!valid_company(value)){ puts("Invalid company."); rejected++; continue; }
                safe_copy(tmp.company,value,MAX_STR);
            }else if(strcmp(field,"ContactPerson")==0){
                if(!valid_contact(value)){ puts("Invalid contact."); rejected++; continue; }
                safe_copy(tmp.contact,value,MAX_STR);
            }else if(strcmp(field,"PhoneNumber")==0){
                if(value[0]=='\0'){ tmp.phone[0]='\0'; }
                else { normalize_phone(value); if(!valid_phone_strict(value)){ puts("Invalid phone."); rejected++; continue; } safe_copy(tmp.phone,value,MAX_PHONE); }
            }else if(strcmp(field,"Email")==0){
                if(value[0]=='\0'){ tmp.email[0]='\0'; }
                else { if(!valid_email_strict(value)){ puts("Invalid email."); rejected++; continue; } safe_copy(tmp.email,value,MAX_STR); }
            }else { puts("Unknown field."); continue; }

            if(!has_at_least_one_contact(&tmp)){ puts("Need at least one of Phone/Email."); rejected++; continue; }
            if(is_duplicate(&tmp, i)){ puts("Duplicate after update."); rejected++; continue; }

            *p = tmp; updated++;
        }
    }
    if(updated){ save_csv(); printf("Updated %d record(s).\n", updated); }
    else if(rejected) puts("No records updated (validation/duplicate failed).");
    else puts("No records updated.");
}

/* ----- delete: multi-select + confirm ----- */
void delete_user(void){
    char ident[128];
    printf("Identifier to delete (company/contact/phone/email): ");
    if(!fgets(ident,sizeof(ident),stdin)){ return; }
    rstrip(ident); trim(ident);

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
    char sel[256]; if(!fgets(sel,sizeof(sel),stdin)) return; rstrip(sel); trim(sel);

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
    rstrip(ident); trim(ident);

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
    char sel[256]; if(!fgets(sel,sizeof(sel),stdin)) return; rstrip(sel); trim(sel);

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
