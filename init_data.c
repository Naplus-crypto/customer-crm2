#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void) {
    // สร้างโฟลเดอร์ data ถ้ายังไม่มี
    struct stat st = {0};
    if (stat("data", &st) == -1) {
        mkdir("data", 0700);
    }

    FILE *f = fopen("data/customers.csv", "w");
    if (!f) {
        perror("cannot create customers.csv");
        return 1;
    }

    fprintf(f, "CompanyName,ContactPerson,PhoneNumber,Email\n");
    fprintf(f, "Tech Solutions,John Doe,555-1234,john@techsol.com\n");
    fprintf(f, "Business Corp,Jane Smith,555-5678,jane@businesscorp.com\n");
    fprintf(f, "Rocket Co,Rita Ray,0812345678,rita@rocket.io\n");
    fprintf(f, "Alpha Inc,Alice Chan,0811111111,alice@alpha.com\n");
    fprintf(f, "Beta Ltd,Bob Lee,0822222222,bob@beta.com\n");
    fprintf(f, "Gamma LLC,Grace Kim,0833333333,grace@gamma.com\n");
    fprintf(f, "Delta Co,David Park,0844444444,david@delta.com\n");
    fprintf(f, "Omega Corp,Oscar Tan,0855555555,oscar@omega.com\n");
    fprintf(f, "Nova Ltd,Nina Wong,0866666666,nina@nova.com\n");
    fprintf(f, "Prime Inc,Paul Choi,0877777777,paul@prime.com\n");
    fprintf(f, "Zenith Co,Zara Ali,0888888888,zara@zenith.com\n");
    fprintf(f, "Apex LLC,Andy Lim,0899999999,andy@apex.com\n");
    fprintf(f, "Echo Corp,Eva Sun,0901234567,eva@echo.com\n");
    fprintf(f, "Hyper Ltd,Hank Zhao,0912345678,hank@hyper.com\n");
    fprintf(f, "Quantum Co,Quinn Yu,0923456789,quinn@quantum.com\n");

    fclose(f);
    printf("✓ data/customers.csv created with sample data.\n");
    return 0;
}
