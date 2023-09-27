CC 				:= clang
WASI_SDK_CLANG	:= $(HOME)/wasi-sdk-20.0/bin/clang
MKDIR 			?= mkdir 
build_dir 		?=
libc 			?=
libc_path 		:= $(subst build/,, $(dir $(libc)))
include 		?=

wasm 			:= $(build_dir)/main.wasm
obj 			:= $(build_dir)/main.o

.PHONY: build
build: $(wasm) $(obj)

$(wasm): main.c
	$(MKDIR) -p $(@D)
	$(WASI_SDK_CLANG) -nostdlib -I$(libc_path) -I$(include) $^ $(libc) -o $@

$(obj): $(wasm)
$(obj): main.S
	$(MKDIR) -p $(@D)
	$(CC) -D__wasm_path__='"$(wasm)"' --target=$(ARCH) -c -o $@ $<