# ---- Compiler & Flags ----
CC      := gcc
CFLAGS  := -std=c11 -O2 -Wall -Wextra -pedantic

# ---- Files ----
BIN     := crm
SRC     := main.c customer_manager.c

UNIT_SRC := tests/test_unit.c
UNIT_BIN := tests/test_unit

E2E_SRC  := tests/test_e2e.c
E2E_BIN  := tests/test_e2e
E2E_IN   := tests/e2e_input.txt
E2E_OUT  := tests/e2e_output.txt

.PHONY: all run clean test e2e e2e-bin help

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN)

# ---------- Unit test ----------
$(UNIT_BIN): $(UNIT_SRC) customer_manager.c
	# ใส่ -I.. เผื่อมีการ include ไฟล์จากโฟลเดอร์แม่
	$(CC) $(CFLAGS) -I.. -o $(UNIT_BIN) $(UNIT_SRC)

test: $(UNIT_BIN)
	./$(UNIT_BIN)

# ---------- E2E binary ----------
$(E2E_BIN): $(E2E_SRC)
	$(CC) $(CFLAGS) -I.. -o $(E2E_BIN) $(E2E_SRC)

e2e-bin: $(E2E_BIN) $(BIN)
	./$(E2E_BIN)

# ---------- E2E via input script ----------
e2e: $(BIN)
	@echo "Running E2E (scripted input)..."
	@./$(BIN) < $(E2E_IN) > $(E2E_OUT) || true
	@grep -q "Added\."         $(E2E_OUT) && echo "[E2E] Add OK"    || (echo "[E2E] Add FAIL"; exit 1)
	@grep -q "Updated"         $(E2E_OUT) && echo "[E2E] Update OK" || (echo "[E2E] Update FAIL"; exit 1)
	@grep -q "Marked Inactive" $(E2E_OUT) && echo "[E2E] Delete OK" || (echo "[E2E] Delete FAIL"; exit 1)
	@grep -q "Bye!"            $(E2E_OUT) && echo "[E2E] Exit OK"   || (echo "[E2E] Exit FAIL"; exit 1)
	@echo "[E2E] Passed"

clean:
	rm -f $(BIN) $(UNIT_BIN) $(E2E_BIN) $(E2E_OUT) tests/mock_input.txt tests/test_unit.csv

help:
	@echo "make         # build app"
	@echo "make run     # run app"
	@echo "make test    # build+run unit tests"
	@echo "make e2e     # run E2E via input script"
	@echo "make e2e-bin # build+run tests/test_e2e (binary)"
	@echo "make clean   # remove artifacts"
