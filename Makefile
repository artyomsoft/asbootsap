BUILD_DIR := ./build
SRC_DIRS := ./src
LIBDIR := /usr/lib
ARCH := x86_64

TARGET_NAME := asbootsap

OBJCOPY := objcopy
CC := gcc
LD := ld

FORMAT := efi-app-x86-64

SECTIONS := .text .sdata .data .dynamic .dynsym .rel .rela .reloc

CRT0 := $(shell find $(LIBDIR) -name crt0-efi-$(ARCH).o 2>/dev/null | tail -n1)

LDSCRIPT := $(shell find $(LIBDIR) -name elf_$(ARCH)_efi.lds 2>/dev/null | tail -n1)

# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise,
# but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
SRCS_ASM := $(shell find $(SRC_DIRS) -name '*.asm')

OBJS_ASM := $(patsubst ./src/%.asm,$(BUILD_DIR)/%.o,$(SRCS_ASM))
OBJS := $(patsubst ./src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PRECIOUS: $(OBJS)
.PRECIOUS: $(OBJS_ASM)
.PRECIOUS: $(BUILD_DIR)/%.so

# String substitution (suffix version without %).
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
GNU_EFI_DIRS := /usr/include/efi /usr/include/efi/$(ARCH)
INC_DIRS := $(INC_DIRS) $(GNU_EFI_DIRS)

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) \
	-MMD \
	-MP

CFLAGS := -fshort-wchar \
	-DGNU_EFI_USE_MS_ABI \
	-ffreestanding \
	-mno-red-zone \
	-Wall \
	-Werror \
	-fPIC \
	-O2


LDFLAGS=-T $(LDSCRIPT) \
	-Bsymbolic \
	-shared \
	-nostdlib \
	-znocombreloc \
	-L$(LIBDIR) \
	$(CRT0)

all: $(BUILD_DIR)/$(TARGET_NAME).efi
	mkdir -p ./build

deploy: all
	mkdir -p ./esp/efi/boot
	cp $(BUILD_DIR)/$(TARGET_NAME).efi ./esp/efi/boot/BOOTX64.EFI

start: all deploy
	./start-qemu.sh

$(BUILD_DIR)/%.efi: $(BUILD_DIR)/%.so
	$(OBJCOPY) $(foreach sec,$(SECTIONS),-j $(sec)) --target=$(FORMAT) -S $< $@

$(BUILD_DIR)/%.so: $(OBJS) $(OBJS_ASM)
	$(LD) $(LDFLAGS) -o $@ $^ -lgnuefi -lefi

# Build step for C source
$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.asm
	mkdir -p $(dir $@)
	nasm -g -f elf64 -l $@.lst $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
