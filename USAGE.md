# Customer CRM – ตัวอย่างการใช้งาน CLI

ไฟล์นี้รวมตัวอย่างการใช้งานครบทุกเมนูของโปรแกรม Customer CRM (CSV)

---

## 0) เริ่มรันโปรแกรม จะเห็นเมนู
```
==== Customer CRM ===
1) List all
2) Add
3) Search
4) Update field
5) Delete (soft)
6) Exit
7) List inactive
8) Restore
9) Run Unit Test
10) Run E2E Test
Choose [1-10]:
```

---

## 1) List all — แสดงข้อมูลทั้งหมด
```
Choose [1-10]: 1

CompanyName           | ContactPerson        | PhoneNumber      | Email
-------------------------------------------------------------------------------
Tech Solutions        | John Doe             | 0812345678       | john@techsol.com
Business Corp         | Jane Smith           | 0823456789       | jane@businesscorp.com
... (รายการอื่น ๆ)
```

---

## 2) Add — เพิ่มลูกค้าใหม่
```
Choose [1-10]: 2
=== Add Customer (STRICT + Practical) ===
กติกา:
 • Company/Contact: 2–80 ตัว, ต้องมี 'ตัวอักษร' อย่างน้อย 1, อนุญาต . - / & ( ) ' , และเว้นวรรค
   ห้ามขึ้นต้น/ลงท้ายด้วยเครื่องหมาย
 • Phone: ตัวเลขล้วน 9–15 หลัก, เริ่มด้วย 0, prefix 02/03/06/08/09 (รองรับ +66 -> 0)
 • Email: มี @ เดียว, โดเมนมีจุด, ไม่มีช่องว่าง/.. , TLD ≥ 2 ตัวอักษร
 • ต้องมีอย่างน้อย 1 ช่องทางติดต่อ (Phone หรือ Email)
----------------------------------------
CompanyName   : ACME Co., Ltd.
ContactPerson : Rita Ray
PhoneNumber   (enter to skip): +66812345678
Email         (enter to skip): rita@acme.co.th
✓ Added.
```

---

## 3) Search — ค้นหาข้อมูล

### 3.1 ค้นหาจากชื่อบริษัท
```
Choose [1-10]: 3
Search by company/contact/phone/email: acme

CompanyName           | ContactPerson        | PhoneNumber      | Email
-------------------------------------------------------------------------------
ACME Co., Ltd.        | Rita Ray             | 0812345678       | rita@acme.co.th
```

### 3.2 ค้นหาจากชื่อผู้ติดต่อ
```
Choose [1-10]: 3
Search by company/contact/phone/email: rita

CompanyName           | ContactPerson        | PhoneNumber      | Email
-------------------------------------------------------------------------------
ACME Co., Ltd.        | Rita Ray             | 0812345678       | rita@acme.co.th
```

### 3.3 ค้นหาจากเบอร์/อีเมล
```
Choose [1-10]: 3
Search by company/contact/phone/email: 08123

CompanyName           | ContactPerson        | PhoneNumber      | Email
-------------------------------------------------------------------------------
ACME Co., Ltd.        | Rita Ray             | 0812345678       | rita@acme.co.th
```

---

## 4) Update field — อัปเดตข้อมูล
```
Choose [1-10]: 4
Identifier to match: acme
Field to update [CompanyName|ContactPerson|PhoneNumber|Email]: PhoneNumber
New value (empty allowed only for Phone/Email): 0890000000
Updated 1 record(s).
```

ตรวจสอบด้วย Search:
```
Choose [1-10]: 3
Search by company/contact/phone/email: 0890

CompanyName           | ContactPerson        | PhoneNumber      | Email
-------------------------------------------------------------------------------
ACME Co., Ltd.        | Rita Ray             | 0890000000       | rita@acme.co.th
```

---

## 5) Delete — *Soft delete*
```
Choose [1-10]: 5
Identifier to delete (company/contact/phone/email): acme
Found 2 matches:
1) ACME Co., Ltd. | Rita Ray | 0812345678 | rita@acme.co.th | Active
2) ACME Co., Ltd. | Bob Park | 023990202  |                  | Active
Delete all (A) or select indices (e.g., 1,3,5): 1,2
You will delete:
 - ACME Co., Ltd. | Rita Ray | 0812345678 | rita@acme.co.th
 - ACME Co., Ltd. | Bob Park | 023990202  |
Confirm delete selected record(s)? [y/N]: y
✓ Marked Inactive (2 record(s)).
```

---

## 6) Exit — ออกจากโปรแกรม
```
Choose [1-10]: 6
Bye!
```

---

## 7) List inactive — แสดงข้อมูลที่ลบแล้ว (soft delete)
```
Choose [1-10]: 7
[Inactive only]
CompanyName           | ContactPerson        | PhoneNumber      | Email
-------------------------------------------------------------------------------
ACME Co., Ltd.        | Rita Ray             | 0812345678       | rita@acme.co.th
ACME Co., Ltd.        | Bob Park             | 023990202        |
```

---

## 8) Restore — กู้คืนข้อมูลที่ลบแล้ว
```
Choose [1-10]: 8
Identifier to restore (company/contact/phone/email): acme
Restore all (A) or select indices (e.g., 2,4): A
Confirm restore ALL matched record(s)? [y/N]: y
✓ Restored (2 record(s)).
```

---

## 9) Run Unit Test — ทดสอบยูนิต
```
Choose [1-10]: 9
Running unit tests...
[Unit] All tests passed!
```

---

## 10) Run E2E Test — ทดสอบ end-to-end
```
Choose [1-10]: 10
Running E2E test...
[E2E] All tests passed!
```
