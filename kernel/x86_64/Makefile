# before make, you have to install gnu-efi(https://github.com/vathpela/gnu-efi)
# clone repository above, and run "make install"

CC := gcc
LD := ld
OBJCOPY := objcopy
QEMU := qemu-system-x86_64

EFIINC := /usr/local/include/efi
CFLAGS := 	-std=c11 -O2 -Wall -Wextra -fPIE -ffreestanding \
			-fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
			-I$(EFIINC)

LIB_PATH := /usr/local/lib
LDFLAGS := 	-shared -Bsymbolic \
			-L$(LIB_PATH) \
			-T$(LIB_PATH)/elf_x86_64_efi.lds

OBJCOPY_FLAGS :=	-j .text -j .sdata -j .data -j .dynamic -j .rodata -j .rel \
	    			-j .rela -j .rel.* -j .rela.* -j .areloc -j .reloc \
					--target efi-app-x86_64

OVMF_PATH := /usr/share/ovmf
QEMU_FLAGS := -bios $(OVMF_PATH)/OVMF.fd -net none -nographic 

.PHONY: build
build: boot-uefi.efi kernel.elf

kernel.elf: CC := clang
kernel.elf: CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=x86_64-elf -ffreestanding -nostdlib -static -masm=intel
kernel.elf: kernel.c
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o $@ $<

boot-uefi.o: boot-uefi.c
	$(CC) $(CFLAGS) -c $< -o $@

boot-uefi.so: boot-uefi.o
	$(LD) $(LDFLAGS) $(LIB_PATH)/crt0-efi-x86_64.o $< -o $@ -lefi -lgnuefi

boot-uefi.efi: boot-uefi.so
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

.PHONY: boot
boot: build
boot: 
	mkdir -p fs/EFI/BOOT
	cp boot-uefi.efi fs/EFI/BOOT/BOOTX64.EFI
	cp kernel.elf fs/kernel.elf
	$(QEMU) $(QEMU_FLAGS) -drive format=raw,file=fat:rw:fs

.PHONY: clean
clean:
	rm -rf *.o *.so *.efi *.elf *.map fs