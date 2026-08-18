# Host-side unit tests for pixl.js firmware pure-logic modules.
#
# Usage:
#   make          build all tests
#   make test     build and run all tests
#   make clean
#
# Requires: a C compiler, OpenSSL dev headers (for the nrf_crypto mock backend).

CC       ?= cc
CFLAGS   ?= -O1 -g -std=gnu2x -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations
OPENSSL  ?= $(shell pkg-config --silence-errors --cflags --libs openssl 2>/dev/null || echo -lcrypto)

APP_SRC  := ../application/src
MOCK_DIR := mocks
BUILD    := build

INCLUDES := -I. -I$(MOCK_DIR) -I$(APP_SRC) -I$(APP_SRC)/ntag -I$(APP_SRC)/mod/vfs \
            -I$(APP_SRC)/mod/df -I$(APP_SRC)/amiidb -I../components/amiitool/include \
            -I../components/amiitool/src -I$(APP_SRC)/hal

# sources under test
SRC_NTAG_STORE := $(APP_SRC)/ntag/ntag_store.c
SRC_NTAG_EMU   := $(APP_SRC)/ntag/ntag_emu_v2.c $(SRC_NTAG_STORE)
SRC_VFS_META   := $(APP_SRC)/mod/vfs/vfs_meta.c
SRC_AMIITOOL   := ../components/amiitool/src/amiibo.c ../components/amiitool/src/keygen.c \
                  ../components/amiitool/src/drbg.c
SRC_HELPER     := $(APP_SRC)/amiibo_helper.c $(SRC_NTAG_STORE) $(SRC_AMIITOOL)

# common mocks/stubs
MOCKS_COMMON := $(MOCK_DIR)/utils2_stub.c $(MOCK_DIR)/vfs_stub.c
MOCK_CRYPTO  := $(MOCK_DIR)/nrf_crypto_mock.c

TEST_BINS := $(addprefix $(BUILD)/,test_ntag_store test_ntag_emu test_vfs_meta test_amiitool test_amiibo_helper)

all: $(TEST_BINS)

test: $(TEST_BINS)
	@fail=0; \
	for t in $(TEST_BINS); do \
	  printf '== %s ==\n' "$$t"; \
	  ./$$t || fail=1; \
	done; \
	exit $$fail

$(BUILD):
	@mkdir -p $(BUILD)

# per-test objects
OBJS_NTAG_STORE := $(BUILD)/test_ntag_store.o $(BUILD)/ntag_store.o $(BUILD)/utils2_stub.o $(BUILD)/vfs_stub.o

OBJS_NTAG_EMU := $(BUILD)/test_ntag_emu.o $(BUILD)/ntag_emu_v2.o $(BUILD)/ntag_store.o \
                 $(BUILD)/hal_nfc_t2t_mock.o $(BUILD)/app_scheduler_mock.o \
                 $(BUILD)/utils2_stub.o $(BUILD)/vfs_stub.o

OBJS_VFS_META := $(BUILD)/test_vfs_meta.o $(BUILD)/vfs_meta.o

OBJS_AMIITOOL := $(BUILD)/test_amiitool.o $(BUILD)/amiibo.o $(BUILD)/keygen.o $(BUILD)/drbg.o \
                 $(BUILD)/nrf_crypto_mock.o

OBJS_HELPER := $(BUILD)/test_amiibo_helper.o $(BUILD)/amiibo_helper.o $(BUILD)/ntag_store.o \
               $(BUILD)/amiibo.o $(BUILD)/keygen.o $(BUILD)/drbg.o $(BUILD)/nrf_crypto_mock.o \
               $(BUILD)/utils2_stub.o $(BUILD)/vfs_stub.o $(BUILD)/db_stub.o

$(BUILD)/test_ntag_store: $(OBJS_NTAG_STORE)
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL)

$(BUILD)/test_ntag_emu: $(OBJS_NTAG_EMU)
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL)

$(BUILD)/test_vfs_meta: $(OBJS_VFS_META)
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL)

$(BUILD)/test_amiitool: $(OBJS_AMIITOOL)
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL)

$(BUILD)/test_amiibo_helper: $(OBJS_HELPER)
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD)/utils2_stub.o: $(MOCK_DIR)/utils2_stub.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/vfs_stub.o: $(MOCK_DIR)/vfs_stub.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/hal_nfc_t2t_mock.o: $(MOCK_DIR)/hal_nfc_t2t_mock.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/app_scheduler_mock.o: $(MOCK_DIR)/app_scheduler_mock.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/nrf_crypto_mock.o: $(MOCK_DIR)/nrf_crypto_mock.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/db_stub.o: $(MOCK_DIR)/db_stub.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# map source files in application/components into flat build objects
$(BUILD)/ntag_store.o: $(APP_SRC)/ntag/ntag_store.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/ntag_emu_v2.o: $(APP_SRC)/ntag/ntag_emu_v2.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/vfs_meta.o: $(APP_SRC)/mod/vfs/vfs_meta.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/amiibo.o: ../components/amiitool/src/amiibo.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/keygen.o: ../components/amiitool/src/keygen.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/drbg.o: ../components/amiitool/src/drbg.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(BUILD)/amiibo_helper.o: $(APP_SRC)/amiibo_helper.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
