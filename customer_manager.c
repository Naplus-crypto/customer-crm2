
"""
Customer CRM with CSV storage.

Columns: CompanyName, ContactPerson, PhoneNumber, Email

Public API (required by assignment):
- open_file
- add_user
- edit_user
- search_user
- delete_user
- display_menu

Notes:
- 'identifier' can be company name or contact person (case-insensitive, partial ok).
- All operations preserve header order and write atomically.
- Phone and email are validated lightly to reduce obvious mistakes.
"""
#include <stdio.h>
