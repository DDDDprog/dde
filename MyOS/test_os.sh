#!/bin/bash

echo "Starting MyOS test..."
echo "====================="

# Run QEMU with our OS for 3 seconds
timeout 3 qemu-system-i386 -cdrom MyOS.iso -m 512M -nographic -no-reboot -no-shutdown &
QEMU_PID=$!

# Wait for QEMU to start
sleep 1

# Send Enter key to GRUB to boot our OS
echo -e "\n" > /proc/$QEMU_PID/fd/0 2>/dev/null || true

# Wait for the timeout
wait $QEMU_PID 2>/dev/null || true

echo "Test completed!"