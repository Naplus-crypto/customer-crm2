#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_ROWS 4096
#define MAX_STR  128
#define MAX_PHONE 64

typedef struct {
    char company[MAX_STR];
    char contact[MAX_STR];
    char phone[MAX_PHONE];
    char email[MAX_STR];
} Customer;

typedef struct {
    Customer items[MAX_ROWS];
    int count;
} CustomerDB;

static CustomerDB db;
static char DATA_PATH[256] = "customers.csv";

/* ===== Utils ===== */
static void rstrip(char *s){
    size_t n = strlen(s);
    while(n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]='\0';
}

/* ===== CSV I/O ===== */
static void ensure_csv_exists(void){
    FILE *f = fopen(DATA_PATH, "r");
    if (f){ fclose(f); return; }
    f = fopen(DATA_PATH, "w");
    if (!f) { perror("create csv"); return; }
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email\n");
    fclose(f);
}
static void load_csv(void){
    db.count = 0;
    FILE *f = fopen(DATA_PATH, "r");
    if (!f){ perror("open csv"); return; }
    char line[512];
    if (!fgets(line, sizeof(line), f)){ fclose(f); return; } // skip header
    while (fgets(line, sizeof(line), f) && db.count < MAX_ROWS){
        rstrip(line);
        if(line[0]=='\0') continue;
        char *tok = strtok(line, ",");
        Customer c = {{0}};
        if(tok){ strncpy(c.company, tok, MAX_STR-1); }
        tok = strtok(NULL, ","); if(tok){ strncpy(c.contact, tok, MAX_STR-1); }
        tok = strtok(NULL, ","); if(tok){ strncpy(c.phone, tok, MAX_PHONE-1); }
        tok = strtok(NULL, ","); if(tok){ strncpy(c.email, tok, MAX_STR-1); }
        db.items[db.count++] = c;
    }
    fclose(f);
}
static void save_csv(void){
    FILE *f = fopen(DATA_PATH, "w");
    if (!f){ perror("save csv"); return; }
    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email\n");
    for (int i=0;i<db.count;i++){
        fprintf(f, "%s,%s,%s,%s\n",
            db.items[i].company, db.items[i].contact,
            db.items[i].phone,   db.items[i].email);
    }
    fclose(f);
}

/* ===== API ===== */
void open_file(void){ ensure_csv_exists(); load_csv(); }

void list_users(void){
    printf("CompanyName           | ContactPerson        | PhoneNumber      | Email\n");
    printf("-------------------------------------------------------------------------------\n");
    for(int i=0;i<db.count;i++){
        printf("%-21s | %-20s | %-16s | %s\n",
            db.items[i].company, db.items[i].contact, db.items[i].phone, db.items[i].email);
    }
}

void add_user(void){
    Customer c = {{0}};
    char buf[256];
    printf("CompanyName: "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.company, buf, MAX_STR-1);
    printf("ContactPerson: "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.contact, buf, MAX_STR-1);
    printf("PhoneNumber: "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.phone, buf, MAX_PHONE-1);
    printf("Email: "); if(!fgets(buf,sizeof(buf),stdin)) return; rstrip(buf); strncpy(c.email, buf, MAX_STR-1);

    if (db.count < MAX_ROWS){
        db.items[db.count++] = c;
        save_csv();
        puts("Added.");
    } else {
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
        Customer *p = &db.items[i];
        if (strstr(p->company,key) || strstr(p->contact,key) ||
            strstr(p->phone,key)   || strstr(p->email,key)){
            printf("%-21s | %-20s | %-16s | %s\n",
                p->company, p->contact, p->phone, p->email);
            found++;
        }
    }
    if(!found) puts("No records found.");
}

void edit_user(void){
    char ident[128], field[32], value[128];
    printf("Identifier to match: ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident);
    printf("Field to update [CompanyName|ContactPerson|PhoneNumber|Email]: ");
    if(!fgets(field,sizeof(field),stdin)) return; rstrip(field);
    printf("New value: "); if(!fgets(value,sizeof(value),stdin)) return; rstrip(value);

    int updated=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        if (strstr(p->company,ident) || strstr(p->contact,ident) ||
            strstr(p->phone,ident)   || strstr(p->email,ident)){
            if(strcmp(field,"CompanyName")==0) strncpy(p->company,value,MAX_STR-1);
            else if(strcmp(field,"ContactPerson")==0) strncpy(p->contact,value,MAX_STR-1);
            else if(strcmp(field,"PhoneNumber")==0) strncpy(p->phone,value,MAX_PHONE-1);
            else if(strcmp(field,"Email")==0) strncpy(p->email,value,MAX_STR-1);
            else { puts("Unknown field."); continue; }
            updated++;
        }
    }
    if(updated){ save_csv(); printf("Updated %d record(s).\n", updated); }
    else puts("No records updated.");
}

void delete_user(void){
    char ident[128];
    printf("Identifier to delete: ");
    if(!fgets(ident,sizeof(ident),stdin)) return; rstrip(ident);
    int kept=0, del=0;
    for(int i=0;i<db.count;i++){
        Customer *p=&db.items[i];
        bool match = strstr(p->company,ident) || strstr(p->contact,ident) ||
                     strstr(p->phone,ident)   || strstr(p->email,ident);
        if(match){ del++; } else { if(i!=kept) db.items[kept]=db.items[i]; kept++; }
    }
    db.count = kept;
    if(del){ save_csv(); printf("Deleted %d record(s).\n", del); }
    else puts("No records deleted.");
}
