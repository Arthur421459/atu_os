# ============================================================
# Diretórios
# ============================================================

SYSROOTDIR = sysroot
SYSROOT_INC = $(SYSROOTDIR)/usr/include
SYSROOT_LIB = $(SYSROOTDIR)/usr/lib

BUILD_DIR = build
TOOLS_DIR = tools


# ============================================================
# Toolchain
# ============================================================

CC = i686-elf-gcc
LD = i686-elf-ld
AR = i686-elf-ar

ASMC = nasm

TOOLS_CC = gcc
DEBUG = i686-elf-gdb


# ============================================================
# Flags
# ============================================================

# Flags usadas por kernel, boot2, drivers e libs do sistema
SYSTEM_CFLAGS = -m32 \
                -std=gnu99 \
                -ffreestanding \
                -ffunction-sections \
                -fdata-sections \
                -O0 \
                -Wall \
                -mno-sse \
                -mno-sse2 \
                -mno-mmx \
                -Wextra \
                -Iinclude \
                -fno-stack-protector \
                -fno-pic \
                -fno-pie \
                -fno-builtin \
                -g


# Flags usadas pelas aplicações e ABI
APP_CFLAGS = -m32 \
             -std=gnu99 \
             -ffreestanding \
             -ffunction-sections \
             -fdata-sections \
             -O0 \
             -Wall \
             -Wextra \
             -mno-sse \
             -mno-sse2 \
             -mno-mmx \
             --sysroot=$(SYSROOTDIR) \
             -I$(SYSROOTDIR)/usr/include \
             -fno-stack-protector \
             -fno-pic \
             -fno-pie \
             -fno-builtin \
             -g


# Assembly ELF32
ASMC_FLAGS = -f elf32


# Linker do kernel / boot2
LD_FLAGS = -m elf_i386 \
           --gc-sections


# Linker das aplicações
APP_LD_FLAGS = --sysroot=$(SYSROOTDIR) \
               -nostdlib \
               -Wl,--gc-sections


# ============================================================
# Sources
# ============================================================

C_KERNEL_SRC = $(wildcard src/kernel/*.c)
ASM_KERNEL_SRC = $(wildcard src/kernel/*.asm)

C_BOOT2_SRC = $(wildcard src/boot2/*.c)
ASM_BOOT2_SRC = $(wildcard src/boot2/*.asm)

ASM_BOOT1_SRC = $(wildcard src/boot1/*.asm)

C_DRIVER_SRC = $(wildcard src/drivers/*.c)
C_LIB_SRC = $(wildcard src/lib/*.c)

C_APPS_SRC = $(wildcard src/apps/*.c)

C_ABI_ATU_SRC = $(wildcard src/abi/atu/*.c)

C_TOOLS_SRC = $(wildcard src/tools/*.c)

ASM_CRT_SRC = $(wildcard src/crtabi/*.asm)


# ============================================================
# Objetos do sistema
# ============================================================

SRC_C = \
	$(C_DRIVER_SRC) \
	$(C_LIB_SRC)

OBJ_C = $(patsubst \
	src/%.c,$(BUILD_DIR)/c/%.o,$(SRC_C))


# Kernel
OBJ_KERNEL_C = $(patsubst \
	src/%.c,$(BUILD_DIR)/c/%.o,$(C_KERNEL_SRC))

OBJ_KERNEL_ASM = $(patsubst \
	src/%.asm,$(BUILD_DIR)/asm/%.o,$(ASM_KERNEL_SRC))


# Boot2
OBJ_BOOT2_C = $(patsubst \
	src/%.c,$(BUILD_DIR)/c/%.o,$(C_BOOT2_SRC))

OBJ_BOOT2_ASM = $(patsubst \
	src/%.asm,$(BUILD_DIR)/asm/%.o,$(ASM_BOOT2_SRC))


# Boot1
OBJ_BOOT1_ASM = $(patsubst \
	src/boot1/%.asm,$(BUILD_DIR)/%.bin,$(ASM_BOOT1_SRC))


# ============================================================
# ABI ATU
# ============================================================

OBJ_ABI_ATU = $(patsubst \
	src/abi/atu/%.c,$(BUILD_DIR)/abi/atu/%.o,$(C_ABI_ATU_SRC))

ABI_ATU_LIB = $(SYSROOT_LIB)/libatu.a


# ============================================================
# CRT
# ============================================================

OBJ_CRT_ASM = $(patsubst \
	src/%.asm,$(BUILD_DIR)/%.o,$(ASM_CRT_SRC))


# ============================================================
# Aplicações
# ============================================================

OBJ_APPS_C = $(patsubst \
	src/apps/%.c,$(BUILD_DIR)/c/apps/%.o,$(C_APPS_SRC))

ELF_APPS_C = $(patsubst \
	$(BUILD_DIR)/c/apps/%.o,rootfs/%,$(OBJ_APPS_C))


# ============================================================
# Ferramentas
# ============================================================

OBJ_TOOLS_C = $(patsubst \
	src/tools/%.c,$(TOOLS_DIR)/%,$(C_TOOLS_SRC))


# ============================================================
# RootFS
# ============================================================

ROOTFS_FILES = $(wildcard rootfs/*)


# ============================================================
# Target padrão
# ============================================================

all: hd.img


# ============================================================
# Compilação C do sistema
#
# kernel
# boot2
# drivers
# lib
# ============================================================

$(BUILD_DIR)/c/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(SYSTEM_CFLAGS) -c $< -o $@


# ============================================================
# Compilação C das aplicações
# ============================================================

$(BUILD_DIR)/c/apps/%.o: src/apps/%.c
	@mkdir -p $(dir $@)
	$(CC) $(APP_CFLAGS) -c $< -o $@


# ============================================================
# Assembly ELF32
# ============================================================

$(BUILD_DIR)/asm/%.o: src/%.asm
	@mkdir -p $(dir $@)
	$(ASMC) $(ASMC_FLAGS) $< -o $@


# ============================================================
# ABI ATU - compilação
# ============================================================

$(BUILD_DIR)/abi/atu/%.o: src/abi/atu/%.c
	@mkdir -p $(dir $@)
	$(CC) $(APP_CFLAGS) -c $< -o $@


# ============================================================
# ABI ATU - biblioteca
# ============================================================

$(ABI_ATU_LIB): $(OBJ_ABI_ATU)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^


# ============================================================
# Headers da ABI ATU
#
# Estrutura esperada:
#
# src/abi/atu/include/atuos/core.h
#
# vira:
#
# sysroot/usr/include/atuos/core.h
# ============================================================

ABI_ATU_HEADERS = $(shell find src/abi/atu/include -type f 2>/dev/null)

ABI_ATU_HEADER_TARGETS = $(patsubst \
	src/abi/atu/include/%,$(SYSROOT_INC)/%,$(ABI_ATU_HEADERS))


$(SYSROOT_INC)/%: src/abi/atu/include/%
	@mkdir -p $(dir $@)
	cp $< $@


# ============================================================
# CRT
# ============================================================

$(BUILD_DIR)/crtabi/%.o: src/crtabi/%.asm
	@mkdir -p $(dir $@)
	$(ASMC) $(ASMC_FLAGS) $< -o $@


# ============================================================
# Kernel
# ============================================================

kernel.bin: \
	$(OBJ_C) \
	$(OBJ_KERNEL_C) \
	$(OBJ_KERNEL_ASM) \
	linker/kernel.ld

	$(LD) $(LD_FLAGS) \
		-T linker/kernel.ld \
		$(OBJ_C) \
		$(OBJ_KERNEL_C) \
		$(OBJ_KERNEL_ASM) \
		-o $@


# ============================================================
# Boot2
# ============================================================

$(BUILD_DIR)/boot2.bin: \
	$(OBJ_C) \
	$(OBJ_BOOT2_C) \
	$(OBJ_BOOT2_ASM) \
	linker/boot2.ld

	$(LD) $(LD_FLAGS) \
		-T linker/boot2.ld \
		$(OBJ_C) \
		$(OBJ_BOOT2_C) \
		$(OBJ_BOOT2_ASM) \
		-o $(BUILD_DIR)/boot2.elf

	objcopy -O binary \
		$(BUILD_DIR)/boot2.elf \
		$(BUILD_DIR)/boot2.bin


# ============================================================
# Boot1
# ============================================================

$(BUILD_DIR)/%.bin: src/boot1/%.asm
	@mkdir -p $(dir $@)
	$(ASMC) -f bin $< -o $@


# ============================================================
# Aplicações
# ============================================================

rootfs/%: \
	$(BUILD_DIR)/c/apps/%.o \
	$(ABI_ATU_LIB) \
	$(OBJ_CRT_ASM) \
	$(ABI_ATU_HEADER_TARGETS)

	@mkdir -p $(dir $@)

	$(CC) \
		$(APP_LD_FLAGS) \
		-T linker/prog.ld \
		$(OBJ_CRT_ASM) \
		$< \
		-latu \
		-L$(SYSROOT_LIB) \
		-o $@


# ============================================================
# Ferramentas do host
# ============================================================

$(TOOLS_DIR)/%: src/tools/%.c
	@mkdir -p $(dir $@)
	$(TOOLS_CC) -g $< -o $@


# ============================================================
# Imagem do disco
# ============================================================

hd.img: \
	kernel.bin \
	$(BUILD_DIR)/boot2.bin \
	$(OBJ_BOOT1_ASM) \
	$(OBJ_TOOLS_C) \
	$(ROOTFS_FILES) \
	$(ELF_APPS_C)

	dd if=/dev/zero of=hd.img bs=1M count=64

	$(TOOLS_DIR)/mkmbr \
		hd.img boot \
		$(BUILD_DIR)/bootmbr.bin

	$(TOOLS_DIR)/mkmbr \
		hd.img part 1 0

	$(TOOLS_DIR)/mkmbr \
		hd.img active 1

	$(TOOLS_DIR)/mkatufs \
		hd.img part 1 \
		$(BUILD_DIR)/boot2.bin \
		$(BUILD_DIR)/bootvbr.bin

	$(TOOLS_DIR)/cpatufs \
		hd.img 1M \
		kernel.bin \
		/kernel.elf

	$(foreach file,$(wildcard rootfs/*),\
		$(TOOLS_DIR)/cpatufs \
		hd.img 1M \
		$(file) \
		/$(notdir $(file));)


# ============================================================
# Executar
# ============================================================

run: all
	qemu-system-i386 \
		-m 512M \
		-audiodev pa,id=snd0 \
		-device sb16,audiodev=snd0 \
		-drive file=hd.img,format=raw,index=0,media=disk


# ============================================================
# ImHex
# ============================================================

hex: all
	flatpak run net.werwolv.ImHex hd.img


# ============================================================
# Triple fault
# ============================================================

triple: all
	qemu-system-i386 \
		-m 512M \
		-audiodev pa,id=snd0 \
		-device sb16,audiodev=snd0 \
		-drive file=hd.img,format=raw,index=0,media=disk \
		-d int \
		-no-reboot \
		-no-shutdown


# ============================================================
# GDB
# ============================================================

gdb: all
	qemu-system-i386 \
		-audiodev pa,id=snd0 \
		-device sb16,audiodev=snd0 \
		-drive file=hd.img,format=raw,index=0,media=disk \
		-s \
		-S &

	sleep 0.2

	$(DEBUG) \
		-ex 'target remote localhost:1234' \
		./kernel.bin


# ============================================================
# Limpeza
# ============================================================

clean:
	rm -rf \
		$(BUILD_DIR) \
		$(TOOLS_DIR) \
		kernel.bin \
		hd.img \
		$(ELF_APPS_C)


# ============================================================
# Phony
# ============================================================

.PHONY: all run hex triple gdb clean
