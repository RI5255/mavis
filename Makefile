# commands
CC := clang
WASI_SDK_CLANG := $(HOME)/wasi-sdk-20.0/bin/clang
LD := ld.lld
MKDIR := mkdir
CP := cp
RM := rm
OBJCOPY  := llvm-objcopy
QEMU := qemu-system-riscv32
WAT2WASM := wat2wasm

# build settings
TOP_DIR := $(shell pwd)
ARCH := riscv32
BUILD_DIR ?= $(TOP_DIR)/build

kernel_elf = $(BUILD_DIR)/kernel.elf

# flags
CFLAGS :=-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib
CFLAGS += -I$(TOP_DIR)
CFLAGS += -Ikernel/$(ARCH)/include
QEMUFLAGS := -machine virt -bios none -nographic -serial mon:stdio --no-reboot

.PHONY: build
build: servers $(kernel_elf)

arch_obj := $(BUILD_DIR)/kernel/$(ARCH).o

# object files required to build the kernel
objs := $(addprefix $(BUILD_DIR)/kernel/, kernel.o common.o buffer.o list.o module.o vm.o task.o memory.o ipc.o) \
		$(arch_obj)

# rules for building kernel
linker_script := $(BUILD_DIR)/kernel/kernel.ld
$(kernel_elf): OBJS := $(objs)
$(kernel_elf): OBJS += $(addprefix $(BUILD_DIR)/servers/, shell/main.o vm/main.o)

$(kernel_elf): LDFLAGS := -T$(linker_script)
$(kernel_elf): $(objs) $(linker_script)
	$(LD) $(LDFLAGS) -Map $(@:.elf=.map) -o $@ $(OBJS)

$(arch_obj): $(addprefix $(BUILD_DIR)/kernel/$(ARCH)/, boot.o common.o task.o trap.o uart.o)
	$(MKDIR) -p $(@D)
	$(LD) -r -o $@ $^ 

$(BUILD_DIR)/%.o: %.c
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# linker script for kernel
$(BUILD_DIR)/kernel/kernel.ld: kernel/kernel.ld
	$(MKDIR) -p $(@D)
	$(CP) $< $@

# library used by servers
lib := $(BUILD_DIR)/lib/lib.a
$(lib):
	build_dir=$(BUILD_DIR)/lib make -C lib

# rulues for building servers
.PHONY: hello
hello: $(lib)
	build_dir=$(BUILD_DIR)/servers/hello lib=$(lib) include=$(TOP_DIR) make build -C servers/hello

.PHONY: shell
shell: $(lib)
	build_dir=$(BUILD_DIR)/servers/shell lib=$(lib) include=$(TOP_DIR) make build -C servers/shell

.PHONY: vm
vm:	hello $(lib)
	build_dir=$(BUILD_DIR)/servers/vm lib=$(lib) include=$(TOP_DIR) make build -C servers/vm

.PHONY: servers
servers: hello shell vm

# run qemu
.PHONY: run
run: build
	$(QEMU) $(QEMUFLAGS) -kernel $(kernel_elf)

# cleanup
.PHONY: clean
clean:
	$(RM) -rf $(BUILD_DIR)