WASI_SDK_CLANG	?= $(HOME)/wasi-sdk-20.0/bin/clang
MKDIR 			?= mkdir 
build_dir 		?=
libc 			?=
libc_path 		:= $(subst build/,, $(dir $(libc)))
include 		?=

wasm 			:= $(build_dir)/main.wasm

.PHONY: build
build: $(wasm)

$(wasm): main.c
	$(MKDIR) -p $(@D)
	$(WASI_SDK_CLANG) -nostdlib -I$(libc_path) -I$(include) $^ $(libc) -o $@