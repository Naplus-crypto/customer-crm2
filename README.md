# Customer CRM (CSV)
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
