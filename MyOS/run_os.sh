#!/bin/bash

echo "MyOS - Professional Operating System"
echo "===================================="
echo ""

# Build the OS
echo "Building MyOS..."
make clean && make all && make iso

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Build successful! MyOS.iso created."
echo ""

# Show options
echo "Choose how to run MyOS:"
echo "1) Run with VGA output (requires X11/GUI)"
echo "2) Run with serial output only (text mode)"
echo "3) Run and save serial log to file"
echo "4) Exit"
echo ""

read -p "Enter choice (1-4): " choice

case $choice in
    1)
        echo "Starting MyOS with VGA output..."
        echo "Note: This requires a GUI environment"
        qemu-system-i386 -cdrom MyOS.iso -m 512M
        ;;
    2)
        echo "Starting MyOS with serial output..."
        echo "Press Ctrl+A then X to exit QEMU"
        echo ""
        qemu-system-i386 -cdrom MyOS.iso -m 512M -nographic -serial mon:stdio
        ;;
    3)
        echo "Starting MyOS and logging to serial.log..."
        timeout 10 qemu-system-i386 -cdrom MyOS.iso -m 512M -nographic -serial file:serial.log -no-reboot -no-shutdown
        echo ""
        echo "Serial output:"
        echo "=============="
        cat serial.log
        ;;
    4)
        echo "Exiting..."
        exit 0
        ;;
    *)
        echo "Invalid choice!"
        exit 1
        ;;
esac