#!/bin/bash

echo "Testing MyOS boot..."

# Create a temporary expect script to interact with QEMU
cat > /tmp/qemu_test.exp << 'EOF'
#!/usr/bin/expect -f
set timeout 10

spawn qemu-system-i386 -cdrom MyOS.iso -m 512M -nographic -no-reboot -no-shutdown

# Wait for GRUB menu
expect {
    "highlighted entry will be executed automatically" {
        # GRUB will auto-boot, just wait
        sleep 3
    }
    timeout {
        send "\r"
        sleep 3
    }
}

# Wait for kernel output or timeout
expect {
    "MyOS" {
        puts "SUCCESS: MyOS kernel started!"
        sleep 2
    }
    "Kernel" {
        puts "SUCCESS: Kernel output detected!"
        sleep 2
    }
    timeout {
        puts "TIMEOUT: No kernel output detected"
    }
}

# Send Ctrl+C to quit
send "\003"
expect eof
EOF

chmod +x /tmp/qemu_test.exp

# Check if expect is available
if command -v expect >/dev/null 2>&1; then
    /tmp/qemu_test.exp
else
    echo "expect not available, running basic test..."
    timeout 5 qemu-system-i386 -cdrom MyOS.iso -m 512M -nographic -no-reboot -no-shutdown
fi

rm -f /tmp/qemu_test.exp
echo "Test completed."