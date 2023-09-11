CFLAGS 		:= -nostdlib -I$(HOME)/mavis
top_dir 	:= $(shell pwd)
build_dir 	?= $(top_dir)
srcs 		:= $(notdir $(wildcard $(top_dir)/*.c))

.PHONY: wasm-libc
wasm-libc: CC := $(HOME)/wasi-sdk-20.0/bin/clang
wasm-libc: CFLAGS += -D__WASM_LIBC__
wasm-libc: build

.PHONY: libc
libc: CC := clang
libc: CFLAGS += -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding
libc: build

.PHONY: build
build: $(build_dir)/libc.a

$(build_dir)/libc.a: $(addprefix $(build_dir)/, $(srcs:.c=.o))
	mkdir -p $(@D)
	llvm-ar crs $@ $^

$(build_dir)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<