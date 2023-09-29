# commands
CC := clang
WASI_SDK_CLANG := $(HOME)/wasi-sdk-20.0/bin/clang
LD := ld.lld
MKDIR := mkdir
CP := cp
RM := rm
OBJCOPY  := llvm-objcopy

# build settings
TOP_DIR := $(shell pwd)
ARCH ?= riscv64
BUILD_DIR ?= $(TOP_DIR)/build

kernel_elf = $(BUILD_DIR)/kernel.elf

QEMU := qemu-system-$(ARCH)

# flags
CFLAGS :=-std=c11 -O2 -g3 -Wall -Wextra --target=$(ARCH) -mcmodel=medany -ffreestanding -nostdlib -static -fuse-ld=lld -masm=intel
CFLAGS += -I$(TOP_DIR)
CFLAGS += -Ikernel/$(ARCH)/include
QEMUFLAGS := -machine virt -bios none -nographic -serial mon:stdio --no-reboot

ifeq ($(ARCH),x86_64)
	ARCH_OBJS := $(addprefix $(BUILD_DIR)/kernel/$(ARCH)/, boot.o common.o uart.o interrupt.o segment.o asm.o task.o)
else
	ARCH_OBJS := $(addprefix $(BUILD_DIR)/kernel/$(ARCH)/, boot.o common.o task.o trap.o uart.o)
endif

.PHONY: build
build: servers $(kernel_elf)

arch_obj := $(BUILD_DIR)/kernel/$(ARCH).o

# object files required to build the kernel
objs := $(addprefix $(BUILD_DIR)/kernel/, kernel.o buffer.o list.o module.o vm.o task.o memory.o ipc.o) \
		$(arch_obj)

# libc
libc := $(BUILD_DIR)/lib/libc/libc.a

.PHONY: libc
libc: 
	build_dir=$(BUILD_DIR)/lib/libc ARCH=$(ARCH) make libc -B -C lib/libc

# rules for building kernel
linker_script := $(BUILD_DIR)/kernel/kernel.ld
$(kernel_elf): libc
$(kernel_elf): OBJS := $(objs)
$(kernel_elf): OBJS += $(addprefix $(BUILD_DIR)/servers/, shell/main.o vm/main.o fs/main.o)
$(kernel_elf): $(objs) $(linker_script)
	$(CC) $(CFLAGS) -Wl,-T$(linker_script) -Wl,-Map=$(@:.elf=.map) -o $@ $(libc) $(OBJS)

$(arch_obj): $(ARCH_OBJS)
	$(MKDIR) -p $(@D)
	$(LD) -r -o $@ $^ 

$(BUILD_DIR)/%.o: %.c
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) -Ilib/libc -c -o $@ $<

# linker script for kernel
$(BUILD_DIR)/kernel/kernel.ld: kernel/$(ARCH)/kernel.ld
	$(MKDIR) -p $(@D)
	$(CP) $< $@

# library used by servers
.PHONY: wasm-libc
wasm-libc:
	build_dir=$(BUILD_DIR)/lib/libc make wasm-libc -B -C lib/libc

# rulues for building servers
.PHONY: hello
hello:
	build_dir=$(BUILD_DIR)/servers/hello libc=$(libc) include=$(TOP_DIR) ARCH=$(ARCH) make build -C servers/hello

.PHONY: shell
shell:
	build_dir=$(BUILD_DIR)/servers/shell libc=$(libc) include=$(TOP_DIR) ARCH=$(ARCH) make build -C servers/shell

.PHONY: vm
vm:
	build_dir=$(BUILD_DIR)/servers/vm libc=$(libc) include=$(TOP_DIR) ARCH=$(ARCH) make build -C servers/vm

image_path = $(BUILD_DIR)/fs/image

.PHONY: image
image: hello
image: $(image_path)
$(image_path): dirs = $(addprefix $(BUILD_DIR)/servers/, hello)
$(image_path): 
	$(MKDIR) -p $(@D)
	python3 tools/mkimage.py $(dirs) -o $(image_path)

.PHONY: fs
fs: image
	build_dir=$(BUILD_DIR)/servers/fs libc=$(libc) include=$(TOP_DIR) image_path=$(image_path) ARCH=$(ARCH) make build -C servers/fs

.PHONY: servers
servers: wasm-libc
servers: hello shell vm fs

# run qemu
.PHONY: run
run: build
	$(QEMU) $(QEMUFLAGS) -kernel $(kernel_elf)

# cleanup
.PHONY: clean
clean:
	$(RM) -rf $(BUILD_DIR)