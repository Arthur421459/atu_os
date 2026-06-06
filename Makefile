BUILD_DIR = build
SRC_DIR = src
CC = gcc
CC_FLAGS = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude
ASMC = nasm
ASMC_FLAGS = -f elf32
C_KERNEL_SRC = $(wildcard src/kernel/*.c)
ASM_KERNEL_SRC = $(wildcard src/kernel/*.asm)
C_DRIVER_SRC = $(wildcard src/drivers/*.c)
#LIB_SRC    = $(wildcard src/lib/*.c)

SRC_C = $(C_KERNEL_SRC) $(C_DRIVER_SRC)
SRC_ASM = $(ASM_KERNEL_SRC)
OBJ_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(SRC_C))
OBJ_ASM = $(patsubst src/%.asm, $(BUILD_DIR)/asm/%.o, $(SRC_ASM))
all: kernel.bin hd.img

kernel.bin: $(OBJ_C) $(OBJ_ASM)
	ld -m elf_i386 -T linker/linker.ld $(OBJ_ASM) $(OBJ_C) -o kernel.bin

$(BUILD_DIR)/c/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(BUILD_DIR)/asm/%.o: src/%.asm
	@mkdir -p $(dir $@)
	$(ASMC) $(ASMC_FLAGS) $< -o $@

run: all
	qemu-system-i386 -kernel kernel.bin -drive file=hd.img,format=raw,index=0,media=disk

clean:
	rm -rf $(BUILD_DIR) kernel.bin

.PHONY: all run clean