# Customer CRM (CSV)

**ฟีเจอร์หลัก**
- บันทึก/อ่านข้อมูลลูกค้าจาก CSV (CompanyName, ContactPerson, PhoneNumber, Email)
- เพิ่ม (add), ค้นหา (search), อัพเดต (update), ลบ (delete)
- เมนูแสดงผลใช้งานผ่าน CLI (display_menu)
- ไฟล์ CSV มีข้อมูลอย่างน้อย 15 รายการ (ดู `data/customers.csv`)

**ฟังก์ชันหลักใน customer_manager.py**
- open_file() → เปิด/โหลดไฟล์ CSV
- add_user() → เพิ่มข้อมูลลูกค้าใหม่
- search_user() → ค้นหาจากชื่อบริษัทหรือผู้ติดต่อ
- edit_user() → อัพเดทข้อมูล (เช่น เบอร์โทรศัพท์)
- delete_user() → ลบลูกค้า
- display_menu() → แสดงเมนู CLI ให้ผู้ใช้เลือก

**โครงสร้างไฟล์ CSV**
- มี 4 column: CompanyName, ContactPerson, PhoneNumber, Email
- ต้องมีข้อมูล ≥15 แถว (เช่น Tech Solutions, John Doe, …)

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

**CLI (ตัวอย่าง)**
```
==== Customer CRM ===
1) List all
2) Add
3) Search
4) Update
5) Delete
6) Exit
Choose [1-6]: 3
Search by company/contact: Tech

CompanyName    | ContactPerson | PhoneNumber | Email
--------------------------------------------------------
Tech Solutions | John Doe      | 555-1234    | john@techsol.com

```
