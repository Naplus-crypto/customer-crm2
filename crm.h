
#ifndef CRM_H
#define CRM_H

#include <stdbool.h>

#define MAX_STR 128
#define MAX_PHONE 64
#define MAX_CUSTOMERS 2048

typedef struct {
    char company[MAX_STR];
    char contact[MAX_STR];
    char phone[MAX_PHONE];
    char email[MAX_STR];
} Customer;

typedef struct {
    Customer items[MAX_CUSTOMERS];
    int count;
} CustomerList;

// CRUD & Search
bool add_customer(CustomerList* cl, const char* company, const char* contact, const char* phone, const char* email);
int search_customers(const CustomerList* cl, const char* query, int* out_indices, int max_out);
int update_field(CustomerList* cl, const char* identifier, const char* field, const char* new_value);
int delete_by_identifier(CustomerList* cl, const char* identifier);

// utils
void print_table(const CustomerList* cl);
void print_matches(const CustomerList* cl, const int* idx, int n);

#endif
