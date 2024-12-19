LOADER_TARGET?=RELEASE
APP=asbootsap
ARCH = x86_64
LIBDIR=/usr/lib
FORMAT=efi-app-x86-64

OBJCOPY=objcopy

INCDIR := /usr/include

# gnuefi sometimes installs these under a gnuefi/ directory, and sometimes not
CRT0 := $(shell find $(LIBDIR) -name crt0-efi-$(ARCH).o 2>/dev/null | tail -n1)
LDSCRIPT := $(shell find $(LIBDIR) -name elf_$(ARCH)_efi.lds 2>/dev/null | tail -n1)

CFLAGS= -I$(INCDIR)/efi -I$(INCDIR)/efi/$(ARCH) \
		-fshort-wchar \
		-DGNU_EFI_USE_MS_ABI \
		-ffreestanding \
		-mno-red-zone \
		-Wall -Werror \
 		-fPIC

CFLAGS_DEBUG = -g -ggdb3 -O0 -DEFI_DEBUG

CFLAGS_RELEASE = -O2

SECTIONS = .text .sdata .data .dynamic .dynsym .rel .rela .reloc

ifeq ($(LOADER_TARGET),RELEASE)
CFLAGS += $(CFLAGS_RELEASE)
else ifeq ($(LOADER_TARGET),DEBUG)
CFLAGS += $(CFLAGS_DEBUG)
#-DEFI_DEBUG
else
$(error Target not supported: $(LOADER_TARGET))
endif

%.efi: %.so
	$(OBJCOPY) $(foreach sec,$(SECTIONS),-j $(sec)) --target=$(FORMAT) -S $*.so $@

%.debug: %.so
	$(OBJCOPY) --only-keep-debug $*.so $@

LDFLAGS=-T $(LDSCRIPT) -Bsymbolic -shared -nostdlib -znocombreloc \
		-L$(LIBDIR) $(CRT0)

OBJS = main.o \
	bootloader.o \
	configparser.o \
	common.o \
	memory.o \
	filesystems.o \
	gdtutils.o \
	linuxboot64.o \
	efihandover.o \
	chainload.o \
	debugutils.o

all: $(APP).efi $(APP).debug

gdtutils.o: gdtutils.asm
	nasm -g -f elf64 -l gdtutils.lst gdtutils.asm 

$(APP).efi: $(APP).so

$(APP).so: $(OBJS) gdtutils.o
	$(LD) $(LDFLAGS) -o $@ $^ -lgnuefi -lefi $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

clean:
	rm -f $(APP).efi $(APP).so $(APP).debug $(OBJS) gdtutils.o gdtutils.lst

deploy: all
	mkdir -p ../esp/efi/boot
	cp $(APP).efi ../esp/efi/boot/BOOTX64.EFI

startDebug: clean all deploy
	./start-qemu.sh
