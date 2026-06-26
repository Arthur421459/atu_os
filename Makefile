BUILD_DIR = build
TOOLS_DIR = tools
CC = gcc
CC_FLAGS = -m32 -std=gnu99 -ffreestanding -ffunction-sections -fdata-sections -O0 -Wall -mno-sse -mno-sse2 -mno-mmx -Wextra -Iinclude -fno-stack-protector -fno-pic -fno-pie -fno-builtin -g

ASMC = nasm
ASMC_FLAGS = -f elf32
LD_FLAGS = -m elf_i386 --gc-sections
C_KERNEL_SRC = $(wildcard src/kernel/*.c)
ASM_KERNEL_SRC = $(wildcard src/kernel/*.asm)

C_BOOT2_SRC = $(wildcard src/boot2/*.c)
ASM_BOOT2_SRC = $(wildcard src/boot2/*.asm)

ASM_BOOT1_SRC = $(wildcard src/boot1/*.asm)

C_DRIVER_SRC = $(wildcard src/drivers/*.c)
C_LIB_SRC  = $(wildcard src/lib/*.c)

C_TOOLS_SRC = $(wildcard src/tools/*.c)


SRC_C = $(C_DRIVER_SRC) $(C_LIB_SRC)
#SRC_ASM

OBJ_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(SRC_C))
#OBJ_ASM

OBJ_KERNEL_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(C_KERNEL_SRC))
OBJ_KERNEL_ASM = $(patsubst src/%.asm, $(BUILD_DIR)/asm/%.o, $(ASM_KERNEL_SRC))

OBJ_BOOT2_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(C_BOOT2_SRC))
OBJ_BOOT2_ASM = $(patsubst src/%.asm, $(BUILD_DIR)/asm/%.o, $(ASM_BOOT2_SRC))

OBJ_BOOT1_ASM = $(patsubst src/boot1/%.asm, $(BUILD_DIR)/%.bin, $(ASM_BOOT1_SRC))

OBJ_TOOLS_C = $(patsubst src/tools/%.c, $(TOOLS_DIR)/%, $(C_TOOLS_SRC))
all: hd.img

hd.img: kernel.bin $(BUILD_DIR)/boot2.bin $(OBJ_BOOT1_ASM) $(OBJ_TOOLS_C)
	dd if=/dev/zero of=hd.img bs=1M count=64
	$(TOOLS_DIR)/mkmbr hd.img boot $(BUILD_DIR)/bootmbr.bin
	$(TOOLS_DIR)/mkmbr hd.img part 1 0
	$(TOOLS_DIR)/mkmbr hd.img active 1

	$(TOOLS_DIR)/mkatufs hd.img part 1 $(BUILD_DIR)/boot2.bin $(BUILD_DIR)/bootvbr.bin
	$(TOOLS_DIR)/cpatufs hd.img 1048576 kernel.bin 0


kernel.bin: $(OBJ_C) $(OBJ_KERNEL_C) $(OBJ_KERNEL_ASM) linker/kernel.ld
	ld $(LD_FLAGS) -T linker/kernel.ld $(OBJ_C) $(OBJ_KERNEL_C) $(OBJ_KERNEL_ASM) -o kernel.bin

$(BUILD_DIR)/boot2.bin: $(OBJ_C) $(OBJ_BOOT2_C) $(OBJ_BOOT2_ASM) linker/boot2.ld
	ld $(LD_FLAGS) -T linker/boot2.ld $(OBJ_C) $(OBJ_BOOT2_C) $(OBJ_BOOT2_ASM) -o $(BUILD_DIR)/boot2.elf
	objcopy -O binary $(BUILD_DIR)/boot2.elf $(BUILD_DIR)/boot2.bin

$(TOOLS_DIR)/%: src/tools/%.c
	@mkdir -p $(dir $@)
	$(CC) -g $< -o $@

$(BUILD_DIR)/%.bin: src/boot1/%.asm
	$(ASMC) -f bin $< -o $@

$(BUILD_DIR)/c/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(BUILD_DIR)/asm/%.o: src/%.asm
	@mkdir -p $(dir $@)
	$(ASMC) $(ASMC_FLAGS) $< -o $@



mkatufs: hd.img
	python src/python/mkatufs.py hd.img
run: all
	qemu-system-i386 -drive file=hd.img,format=raw,index=0,media=disk

gdb: all
	qemu-system-i386 -drive file=hd.img,format=raw,index=0,media=disk -s -S &
	sleep 0.2
	gdb -ex 'target remote localhost:1234' ./kernel.bin
clean:
	rm -rf $(BUILD_DIR) $(TOOLS_DIR) kernel.bin hd.img 
.PHONY: all run gdb clean fstools