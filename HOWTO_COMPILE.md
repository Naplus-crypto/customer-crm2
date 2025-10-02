# วิธีคอมไพล์และรันโปรแกรม Customer CRM (C)

## 1) คอมไพล์ด้วย gcc
ให้แน่ใจว่าในเครื่องคุณติดตั้ง **gcc** แล้ว  
จากโฟลเดอร์โปรเจค (`customer-crm/`) ให้รันคำสั่งนี้:

```bash
gcc main.c customer_manager.c -std=c11 -O2 -Wall -Wextra -pedantic -o crm
```

คำอธิบาย option:
- `-std=c11` → ใช้มาตรฐานภาษา C11
- `-O2` → optimize ระดับปานกลาง (เร็วขึ้น แต่ยัง debug ได้)
- `-Wall -Wextra -pedantic` → เปิด warning ช่วยตรวจโค้ด

เมื่อสำเร็จ จะได้ไฟล์ executable ชื่อ **crm**

---

## 2) รันโปรแกรม
```bash
./crm
```

---

## 3) รันทดสอบ

### 3.1 Unit Test
```bash
gcc tests/test_unit.c customer_manager.c -std=c11 -O2 -Wall -Wextra -pedantic -o test_unit
./test_unit
```

### 3.2 End-to-End (E2E) Test
```bash
./crm < tests/e2e_input.txt > tests/e2e_output.txt
cat tests/e2e_output.txt
```

---

## หมายเหตุ
- ถ้าใช้ Windows → คำสั่ง `./crm` ให้เปลี่ยนเป็น `crm.exe`
- ถ้าต้องการ clean ไฟล์ .o หรือ .exe สามารถลบได้เอง หรือเขียน Makefile เพิ่มเติม
