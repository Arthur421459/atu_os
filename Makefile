BUILD_DIR = build
CC = gcc
CC_FLAGS = -m32 -std=gnu99 -ffreestanding -ffunction-sections -fdata-sections -O2 -Wall -mno-sse -mno-sse2 -mno-mmx -Wextra -Iinclude -fno-stack-protector -fno-pic -fno-pie -fno-builtin
ASMC = nasm
ASMC_FLAGS = -f elf32
LD_FLAGS = -m elf_i386 --gc-sections
C_KERNEL_SRC = $(wildcard src/kernel/*.c)
ASM_KERNEL_SRC = $(wildcard src/kernel/*.asm)

C_BOOT_SRC = $(wildcard src/boot/*.c)
ASM_BOOT_SRC = $(wildcard src/boot/*.asm)

C_DRIVER_SRC = $(wildcard src/drivers/*.c)

SRC_C = $(C_DRIVER_SRC)
#SRC_ASM

OBJ_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(SRC_C))
#OBJ_ASM

OBJ_KERNEL_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(C_KERNEL_SRC))
OBJ_KERNEL_ASM = $(patsubst src/%.asm, $(BUILD_DIR)/asm/%.o, $(ASM_KERNEL_SRC))

OBJ_BOOT_C = $(patsubst src/%.c, $(BUILD_DIR)/c/%.o, $(C_BOOT_SRC))
OBJ_BOOT_ASM = $(patsubst src/%.asm, $(BUILD_DIR)/asm/%.o, $(ASM_BOOT_SRC))

all: kernel.bin $(BUILD_DIR)/boot2.bin hd.img
	python src/python/mkatufs.py hd.img kernel.bin
	python src/python/add.py hd.img $(BUILD_DIR)

kernel.bin: $(OBJ_C) $(OBJ_KERNEL_C) $(OBJ_KERNEL_ASM) linker/kernel.ld
	ld $(LD_FLAGS) -T linker/kernel.ld $(OBJ_C) $(OBJ_KERNEL_C) $(OBJ_KERNEL_ASM) -o kernel.bin

$(BUILD_DIR)/boot2.bin: $(OBJ_C) $(OBJ_BOOT_C) $(OBJ_BOOT_ASM) linker/boot2.ld
	ld $(LD_FLAGS) -T linker/boot2.ld $(OBJ_C) $(OBJ_BOOT_C) $(OBJ_BOOT_ASM) -o $(BUILD_DIR)/boot2.elf
	objcopy -O binary $(BUILD_DIR)/boot2.elf $(BUILD_DIR)/boot2.bin
	
hd.img:
	dd if=/dev/zero of=hd.img bs=1M count=64

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
	rm -rf $(BUILD_DIR) kernel.bin
.PHONY: all run clean