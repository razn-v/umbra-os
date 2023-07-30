CXX=clang
CXX_FLAGS=-c -target x86_64-none-elf -Isrc -Ivendor -std=c++20 -g -mno-red-zone -mno-80387 \
		  -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mcmodel=kernel -fno-stack-protector \
		  -fno-omit-frame-pointer -Wall -Wextra -fno-rtti -fno-exceptions -fno-unwind-tables \
		  -fno-asynchronous-unwind-tables -ffreestanding -nostdlib
AS=nasm
AS_FLAGS=-felf64
LD=ld
LD_FLAGS=-T linker.ld -nostdlib -static --no-dynamic-linker

SRC_DIR=src/kernel
BUILD_DIR=build
FONTS_DIR=fonts
TARGET=umbra.bin
IMAGE_FILE=image.iso
LIMINE_DIR=vendor/limine

SRCS=$(shell find $(SRC_DIR) -type f -name "*.cpp")
SRCS_ASM=$(shell find $(SRC_DIR) -type f -name "*.asm")
FONTS=$(wildcard $(FONTS_DIR)/*.psf)
SUBDIRS=$(shell find $(SRC_DIR) -type d)
BUILD_SUBDIRS=$(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SUBDIRS))

OBJS=$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
OBJS_ASM=$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(SRCS_ASM))
OBJS_PSFU=$(patsubst $(FONTS_DIR)/%.psf,$(BUILD_DIR)/%.o,$(FONTS))

.PHONY: all disk run run-debug monitor clean

all: $(BUILD_DIR) $(BUILD_SUBDIRS) $(BUILD_DIR)/$(TARGET) disk

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	$(AS) $(AS_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: fonts/%.psf
	objcopy -O elf64-x86-64 -B i386 -I binary $< $@

$(BUILD_DIR)/$(TARGET): $(OBJS) $(OBJS_ASM) $(OBJS_PSFU)
	$(LD) $(LD_FLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_SUBDIRS):
	mkdir -p $@

disk: $(BUILD_DIR)/$(TARGET) 
	rm -rf iso_root
	mkdir -p iso_root/EFI/BOOT
	cd base && tar cf ../iso_root/initramfs.tar *
	cd ..
	cp $(BUILD_DIR)/$(TARGET) \
		vendor/limine.cfg $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin iso_root/
	cp $(LIMINE_DIR)/BOOTX64.EFI iso_root/EFI/BOOT/
	cp $(LIMINE_DIR)/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_FILE)
	$(LIMINE_DIR)/limine.exe bios-install $(IMAGE_FILE)

run:
	-killall qemu-system-x86_64
	qemu-system-x86_64 -m 512M -device pvpanic -serial stdio -enable-kvm -d cpu_reset -cdrom $(IMAGE_FILE) &
	vncviewer :5900
	wait

run-debug:
	-killall qemu-system-x86_64
	qemu-system-x86_64 -m 512M -device pvpanic -d cpu_reset -cdrom $(IMAGE_FILE) -s -S -nographic &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR)/$(TARGET)"

monitor:
	qemu-system-x86_64 -m 512M -device pvpanic -serial stdio -d cpu_reset -cdrom $(IMAGE_FILE) \
		-monitor telnet:127.0.0.1:55555,server,nowait

clean:
	rm -rf $(BUILD_DIR)
	rm $(IMAGE_FILE)
