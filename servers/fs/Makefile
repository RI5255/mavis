CC 				:= clang
WASI_SDK_CLANG	?= $(HOME)/wasi-sdk-20.0/bin/clang
MKDIR 			?= mkdir 
build_dir 		?=
libc 			?=
image_path 		?=
libc_path 		:= $(subst build/,, $(dir $(libc)))
include 		?=

image 			:= $(build_dir)/image.o
wasm 			:= $(build_dir)/main.wasm
obj 			:= $(build_dir)/main.o

.PHONY: build
build: $(wasm) $(obj)

$(image): image.S
	$(MKDIR) -p $(@D)
	$(WASI_SDK_CLANG) -D__image_path__='"$(image_path)"' -c -o $@ image.S

$(wasm): main.c $(image)
	$(MKDIR) -p $(@D)
	$(WASI_SDK_CLANG) -nostdlib -I$(libc_path) -I$(include) $^ $(libc) -o $@

$(obj): $(wasm)
$(obj): main.S
	$(MKDIR) -p $(@D)
	$(CC) -D__wasm_path__='"$(wasm)"' --target=riscv32 -c -o $@ $<