# Customer CRM (CSV)

**ฟีเจอร์หลัก**
- บันทึก/อ่านข้อมูลลูกค้าจาก CSV (CompanyName, ContactPerson, PhoneNumber, Email)
- เพิ่ม (add), ค้นหา (search), อัพเดต (update), ลบ (delete)
- เมนูแสดงผลใช้งานผ่าน CLI (display_menu)
- ไฟล์ CSV มีข้อมูลอย่างน้อย 15 รายการ (ดู `data/customers.csv`)

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
