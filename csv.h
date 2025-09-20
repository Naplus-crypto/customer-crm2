
#ifndef CSV_H
#define CSV_H

#include "crm.h"

int load_csv(const char* path, CustomerList* cl);
// overwrite file
int save_csv(const char* path, const CustomerList* cl);
// ensures directory exists isn't handled here; caller supplies valid path.
void ensure_csv_with_header(const char* path);

#endif
