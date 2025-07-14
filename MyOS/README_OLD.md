# MyOS - A Custom Operating System

A simple but functional operating system written in Assembly and C, using GRUB as the bootloader.

## Features

- Multiboot compliant kernel
- VGA text mode output
- Basic memory management
- Interrupt handling
- Keyboard input support
- Simple shell interface

## Building

### Prerequisites

- GCC cross-compiler (i686-elf-gcc)
- NASM assembler
- GRUB
- xorriso (for ISO creation)
- QEMU (for testing)

### Build Commands

```bash
make all        # Build the kernel
make iso        # Create bootable ISO
make run        # Run in QEMU
make clean      # Clean build files
```

## Project Structure

```
MyOS/
├── src/
│   ├── boot/           # Boot assembly code
│   ├── kernel/         # Kernel C code
│   ├── drivers/        # Device drivers
│   └── include/        # Header files
├── build/              # Build output
├── iso/                # ISO creation files
└── Makefile           # Build system
```

## Memory Layout

- 0x00100000: Kernel load address
- 0x00200000: Heap start
- 0xC0000000: Virtual memory mapping

## License

MIT License