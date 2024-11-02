%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
		-j .rela -j .reloc -S --target=$(FORMAT) $*.so $@

OBJCOPY=objcopy

HOST  = $(shell $(CC) -dumpmachine | sed "s/\(-\).*$$//")
ARCH := $(shell $(CC) -dumpmachine | sed "s/\(-\).*$$//")

ifeq ($(ARCH),x86_64)
	LIBDIR=/usr/lib
	FORMAT=efi-app-x86-64
else
	ARCH=ia32
	LIBDIR=/usr/lib
	FORMAT=efi-app-$(ARCH)
endif

INCDIR := /usr/include

# gnuefi sometimes installs these under a gnuefi/ directory, and sometimes not
CRT0 := $(shell find $(LIBDIR) -name crt0-efi-$(ARCH).o 2>/dev/null | tail -n1)
LDSCRIPT := $(shell find $(LIBDIR) -name elf_$(ARCH)_efi.lds 2>/dev/null | tail -n1)

CFLAGS=-I. -I$(INCDIR)/efi -I$(INCDIR)/efi/$(ARCH) \
		-DEFI_FUNCTION_WRAPPER -fPIC -fshort-wchar -ffreestanding \
		-Wall -Ifs/ -Iloaders/ -D$(ARCH) -Werror

ifeq ($(ARCH),ia32)
	ifeq ($(HOST),x86_64)
		CFLAGS += -m32
	endif
endif
ifeq ($(ARCH),x86_64)
	CFLAGS += -mno-red-zone
endif

LDFLAGS=-T $(LDSCRIPT) -Bsymbolic -shared -nostdlib -znocombreloc \
		-L$(LIBDIR) $(CRT0)

IMAGE=efilinux.efi
OBJS = entry.o

all: $(IMAGE)

efilinux.efi: efilinux.so

efilinux.so: $(OBJS) $(FS) $(LOADERS)
	$(LD) $(LDFLAGS) -o $@ $^  -lgnuefi -lefi $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

clean:
	rm -f $(IMAGE) efilinux.so $(OBJS) $(FS) $(LOADERS)
