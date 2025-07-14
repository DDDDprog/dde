# MyOS Makefile
# Professional build system for a custom operating system

# Compiler and tools configuration
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso
BOOT_DIR = $(SRC_DIR)/boot
KERNEL_DIR = $(SRC_DIR)/kernel
DRIVERS_DIR = $(SRC_DIR)/drivers
INCLUDE_DIR = $(SRC_DIR)/include

# Compiler flags
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -I$(INCLUDE_DIR) -ffreestanding -O2

# Assembler flags
ASFLAGS = -f elf32

# Linker flags
LDFLAGS = -m elf_i386 -T linker.ld

# Source files
BOOT_SOURCES = $(wildcard $(BOOT_DIR)/*.asm)
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c)
DRIVER_SOURCES = $(wildcard $(DRIVERS_DIR)/*.c)

# Object files
BOOT_OBJECTS = $(BOOT_SOURCES:$(BOOT_DIR)/%.asm=$(BUILD_DIR)/%.o)
KERNEL_OBJECTS = $(KERNEL_SOURCES:$(KERNEL_DIR)/%.c=$(BUILD_DIR)/%.o)
DRIVER_OBJECTS = $(DRIVER_SOURCES:$(DRIVERS_DIR)/%.c=$(BUILD_DIR)/%.o)

ALL_OBJECTS = $(BOOT_OBJECTS) $(KERNEL_OBJECTS) $(DRIVER_OBJECTS)

# Target kernel binary
KERNEL = $(BUILD_DIR)/kernel.bin

# ISO file
ISO_FILE = MyOS.iso

# Default target
.PHONY: all
all: $(KERNEL)

# Build kernel
$(KERNEL): $(ALL_OBJECTS) linker.ld
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) -o $@ $(sort $(ALL_OBJECTS))
	@echo "Kernel built successfully!"

# Compile assembly files
$(BUILD_DIR)/%.o: $(BOOT_DIR)/%.asm
	@mkdir -p $(BUILD_DIR)
	@echo "Assembling $<..."
	$(AS) $(ASFLAGS) $< -o $@

# Compile C files from kernel
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@

# Compile C files from drivers
$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@

# Create bootable ISO
.PHONY: iso
iso: $(KERNEL)
	@echo "Creating ISO..."
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL) $(ISO_DIR)/boot/
	@cp grub.cfg $(ISO_DIR)/boot/grub/
	@grub-mkrescue -o $(ISO_FILE) $(ISO_DIR) 2>/dev/null || echo "Warning: grub-mkrescue not available"
	@echo "ISO created: $(ISO_FILE)"

# Run in QEMU
.PHONY: run
run: iso
	@echo "Starting QEMU..."
	@qemu-system-i386 -cdrom $(ISO_FILE) -m 512M || echo "QEMU not available, please install qemu-system-i386"

# Run in QEMU with debugging
.PHONY: debug
debug: iso
	@echo "Starting QEMU with debugging..."
	@qemu-system-i386 -cdrom $(ISO_FILE) -m 512M -s -S || echo "QEMU not available"

# Test the OS with serial output
.PHONY: test
test: iso
	@echo "Testing MyOS with serial output..."
	@timeout 8 qemu-system-i386 -cdrom $(ISO_FILE) -m 512M -nographic -serial file:serial.log -no-reboot -no-shutdown || true
	@echo ""
	@echo "Serial output:"
	@echo "=============="
	@cat serial.log 2>/dev/null || echo "No serial output captured"
	@echo ""
	@if grep -q "MyOS kernel started via serial" serial.log 2>/dev/null && grep -q "MyOS kernel halted successfully" serial.log 2>/dev/null; then \
		echo "✅ TEST PASSED: MyOS boots and runs successfully!"; \
	else \
		echo "❌ TEST FAILED: MyOS did not boot properly"; \
	fi

# Clean build files
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(ISO_DIR)/boot
	@rm -f $(ISO_FILE)
	@echo "Clean complete!"

# Install dependencies (Ubuntu/Debian)
.PHONY: install-deps
install-deps:
	@echo "Installing dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y build-essential nasm grub-pc-bin xorriso qemu-system-x86
	@echo "Dependencies installed!"

# Show help
.PHONY: help
help:
	@echo "MyOS Build System"
	@echo "=================="
	@echo "Available targets:"
	@echo "  all          - Build the kernel"
	@echo "  iso          - Create bootable ISO"
	@echo "  run          - Run in QEMU"
	@echo "  test         - Test OS with serial output"
	@echo "  debug        - Run in QEMU with debugging"
	@echo "  clean        - Clean build files"
	@echo "  install-deps - Install build dependencies"
	@echo "  help         - Show this help"

# Prevent make from deleting intermediate files
.PRECIOUS: $(BUILD_DIR)/%.o