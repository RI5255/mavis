CC 			:= $(HOME)/wasi-sdk-20.0/bin/clang

top_dir 	:= $(shell pwd)
build_dir 	?= $(top_dir)
srcs 		:= $(notdir $(wildcard $(top_dir)/*.c))

.PHONY: buid
build: $(build_dir)/libc.a

$(build_dir)/libc.a: $(addprefix $(build_dir)/, $(srcs:.c=.o))
	mkdir -p $(@D)
	llvm-ar crs $@ $^

$(build_dir)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c -nostdlib -o $@ $<