# Customer CRM (CSV)

**ฟีเจอร์หลัก**
- บันทึก/อ่านข้อมูลลูกค้าจาก CSV (CompanyName, ContactPerson, PhoneNumber, Email)
- เพิ่ม (add), ค้นหา (search), อัพเดต (update), ลบ (delete)
- เมนูแสดงผลการใช้งานผ่าน CLI (display_menu)

**ฟังก์ชันหลักใน customer_manager.c**
- open_file() → เปิด/โหลดไฟล์ CSV
- add_user() → เพิ่มข้อมูลลูกค้าใหม่
- search_user() → ค้นหาจากชื่อบริษัทหรือผู้ติดต่อ
- edit_user() → อัพเดทข้อมูล (เช่น เบอร์โทรศัพท์)
- delete_user() → ลบลูกค้า
- display_menu() → แสดงเมนู CLI ให้ผู้ใช้เลือก

**โครงสร้างไฟล์ CSV**
- มี 4 column: CompanyName, ContactPerson, PhoneNumber, Email
- ไฟล์ CSV ต้องมีข้อมูลอย่างน้อย 15 รายการ (ดู `data/customers.csv`)

**โครงสร้างโปรเจค**
```
customer_crm/
├── customer_manager.c    # ฟังก์ชันหลักทั้งหมด
├── main.c                # จุดรันโปรแกรม
├── data/
│   └── customers.csv     # ไฟล์ข้อมูลตัวอย่าง (>= 15 แถว)
└── tests/
    ├── test_unit.c       # Unit tests สำหรับทุกฟังก์ชัน
    └── test_e2e.c        # E2E test จำลองการใช้งานจริง
```

**ตัวอย่างการใช้งาน CLI**

**0) เริ่มรันโปรแกรม จะเห็นเมนู:**
```
==== Customer CRM ===
1) List all
2) Add
3) Search
4) Update
5) Delete
6) Exit
Choose [1-6]:
```

**1) List all — แสดงข้อมูลทั้งหมด**
```
==== Customer CRM ===
1) List all
2) Add
3) Search
4) Update
5) Delete
6) Exit
Choose [1-6]: 1

CompanyName     | ContactPerson | PhoneNumber  | Email
---------------------------------------------------------------
Tech Solutions  | John Doe      | 555-1234     | john@techsol.com
Business Corp   | Jane Smith    | 555-5678     | jane@businesscorp.com
... (รายการอื่น ๆ)
```

**2) Add — เพิ่มลูกค้าใหม่**
```
==== Customer CRM ===
1) List all
2) Add
3) Search
4) Update
5) Delete
6) Exit
Choose [1-6]: 2
CompanyName: Rocket Co
ContactPerson: Rita Ray
PhoneNumber: 0812345678
Email: rita@rocket.io
Added.
```

**3) Search — ค้นหาข้อมูล**
3.1 ค้นหาจากชื่อบริษัท
```
Choose [1-6]: 3
Search by company/contact/phone/email: rocket

CompanyName  | ContactPerson | PhoneNumber  | Email
------------------------------------------------------------
Rocket Co    | Rita Ray      | 0812345678   | rita@rocket.io
```
3.2 ค้นหาจากชื่อผู้ติดต่อ
```
Choose [1-6]: 3
Search by company/contact/phone/email: rita

CompanyName  | ContactPerson | PhoneNumber  | Email
------------------------------------------------------------
Rocket Co    | Rita Ray      | 0812345678   | rita@rocket.io
```
3.3 ค้นหาจากเบอร์/อีเมลก็ได้
```
Choose [1-6]: 3
Search by company/contact/phone/email: 08123

CompanyName  | ContactPerson | PhoneNumber  | Email
------------------------------------------------------------
Rocket Co    | Rita Ray      | 0812345678   | rita@rocket.io
```

**4) Update field — อัปเดตข้อมูล (เช่น เปลี่ยนเบอร์โทร)**
```
Choose [1-6]: 4
Identifier to match (company/contact/phone/email): rocket
Field to update [CompanyName|ContactPerson|PhoneNumber|Email]: PhoneNumber
New value: 0890000000
Updated 1 record(s).
```
ตรวจด้วยการค้นหา:
```
Choose [1-6]: 3
Search by company/contact/phone/email: 0890

CompanyName  | ContactPerson | PhoneNumber  | Email
------------------------------------------------------------
Rocket Co    | Rita Ray      | 0890000000   | rita@rocket.io
```
หมายเหตุ: ถ้าใส่ค่าใหม่ไม่ถูกต้อง (เช่น เบอร์สั้นเกิน, อีเมลไม่ถูก) จะขึ้น error

**5) Delete — ลบข้อมูลลูกค้าที่แมตช์กับ identifier**
```
Choose [1-6]: 5
Identifier to delete (company/contact/phone/email): rocket
Deleted 1 record(s).
```
ลอง List เพื่อยืนยัน:
```
Choose [1-6]: 1
... (ไม่มี Rocket Co แล้ว)
```
**6) Exit — ออกจากโปรแกรม**
```
Choose [1-6]: 6
Bye!
```
