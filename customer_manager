
"""
Customer CRM with CSV storage.

Columns: CompanyName, ContactPerson, PhoneNumber, Email

Public API (required by assignment):
- open_file(path) -> list[dict]
- add_user(path, company, contact, phone, email) -> dict (added row)
- edit_user(path, identifier, field, new_value) -> int (rows updated)
- delete_user(path, identifier) -> int (rows deleted)
- search_user(path, query) -> list[dict]
- display_menu(path) -> None (interactive CLI)

Notes:
- 'identifier' can be company name or contact person (case-insensitive, partial ok).
- All operations preserve header order and write atomically.
- Phone and email are validated lightly to reduce obvious mistakes.
"""
