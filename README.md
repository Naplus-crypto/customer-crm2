# Customer CRM (CSV)

ระบบจัดการลูกค้าผ่านบรรทัดคำสั่ง (CLI) จัดเก็บข้อมูลในไฟล์ CSV รองรับชื่อบริษัทที่มี `,`/`"` (ด้วย CSV quoting) มีการตรวจสอบข้อมูล (validation) แบบ “เข้มแต่ใช้งานจริง” และลบแบบ **Soft Delete** กู้คืนได้

---

## ฟีเจอร์หลัก
- บันทึก/อ่านข้อมูลลูกค้าจาก CSV (`CompanyName, ContactPerson, PhoneNumber, Email, Status`)
- เพิ่ม (add), ค้นหา (search), อัปเดต (update), ลบแบบ *soft delete* (delete) และกู้คืน (restore)
- เมนูใช้งานผ่าน CLI
- รันทดสอบ **Unit Test** และ **E2E Test** จากเมนู

---

## โครงสร้างโปรเจค
```
customer-crm/
├── customer_manager.c   # ฟังก์ชันหลัก CRUD + CSV (validation + soft delete)
├── main.c               # โปรแกรมหลัก + เมนู
├── customers.csv        # ไฟล์ข้อมูลตัวอย่าง (>=15 records)
└── tests/
    ├── test_unit.c      # Unit test (เรียกผ่านเมนู)
    ├── test_e2e.c       # ไฟล์ตัวอย่าง (เมนูจะใช้ e2e_input.txt)
    ├── e2e_input.txt    # Input จำลองสำหรับ E2E
    └── e2e_output.txt   # Output ที่ได้จาก E2E (สร้างตอนรัน)
```

> **หมายเหตุ:** CSV รองรับชื่อที่มี comma/quote เช่น `ACME Co., Ltd.` ด้วยการ quote/escape ตามมาตรฐาน

---

## Validation
- **CompanyName / ContactPerson**
  - ความยาว **2–80** ตัว
  - ต้องมี “**ตัวอักษร**” อย่างน้อย 1 (กันชื่อเป็นตัวเลข/สัญลักษณ์ล้วน)
  - อนุญาตเครื่องหมายที่พบบ่อย: `` . - / & ( ) ' , `` และเว้นวรรค  
    (ถ้ามี `,`/`"` ระบบจะ quote ให้ตอนบันทึก CSV)
  - **ห้ามขึ้นต้น/ลงท้ายด้วยเครื่องหมาย**, ห้าม control char
  - กัน run เครื่องหมายติดกันเกิน 2 ตัว (เช่น `***`, `##??`)
- **PhoneNumber**
  - เก็บเป็น **ตัวเลขล้วน 9–15 หลัก** และต้องเริ่มด้วย `0`
  - prefix ต้องเป็น **02/03/06/08/09**
  - รองรับการป้อน `+66…` แล้วแปลงเป็น `0…` อัตโนมัติ
  - *ใส่ได้หรือเว้นว่างก็ได้*
- **Email**
  - มี `@` เดียว, โดเมนมีจุด, ไม่มีช่องว่าง/`..`, **TLD ≥ 2** ตัวอักษร
  - *ใส่ได้หรือเว้นว่างก็ได้*
- **ต้องมีอย่างน้อยหนึ่งช่องทางติดต่อ** (Phone หรือ Email)
- **กันข้อมูลซ้ำ**
  - ถ้ามี Email → คีย์คือ `(Company, Contact, Email)` ต้องไม่ซ้ำ
  - ถ้า **ไม่มี** Email → คีย์คือ `(Company, Contact, Phone)` ต้องไม่ซ้ำ

---

## ฟังก์ชันหลักใน `customer_manager.c`
- `open_file()` → เปิด/โหลดไฟล์ CSV (ไม่มีไฟล์จะสร้าง header ให้)
- `add_user()` → เพิ่มข้อมูลลูกค้าใหม่ (มีข้อความอธิบายกติกา/แจ้ง error ชัดเจน)
- `search_user()` → ค้นหาตามบริษัท/ผู้ติดต่อ/เบอร์/อีเมล (แบบ contains)
- `edit_user()` → อัปเดตฟิลด์ (CompanyName/ContactPerson/PhoneNumber/Email) พร้อม re-validate และกันซ้ำ
- `delete_user()` → *Soft delete* เลือกได้หลายแถวหรือทั้งหมด พร้อม **ยืนยัน**
- `restore_user()` → กู้คืนข้อมูลที่ถูกลบแบบ soft (เลือกหลายแถว/ทั้งหมด + ยืนยัน)
- `list_users()` / `list_inactive()` → แสดง Active/Inactive
- `run_unit_test()` / `run_e2e_test()` → เรียกยูนิตเทสต์/E2E จากเมนู

---

## โครงสร้างไฟล์ CSV
- คอลัมน์: `CompanyName,ContactPerson,PhoneNumber,Email,Status`
- `Status` เป็น `Active` หรือ `Inactive`
- ต้องมีข้อมูลอย่างน้อย **15 บรรทัด** (ดูตัวอย่างใน `customers.csv`)

---

## ตัวอย่างการใช้งาน CLI

**เริ่มรันโปรแกรม จะเห็นเมนู:**
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
