#!/bin/bash
# build.sh - Build script for CustomOS

echo "========================================="
echo "  Building CustomOS v1.0"
echo "========================================="

# Install dependencies (if needed)
# brew install nasm qemu i686-elf-gcc

# Clean previous builds
rm -f *.o kernel.bin customos.iso
rm -rf isodir

echo "[1/5] Compiling bootloader..."
nasm -f elf32 boot.asm -o boot.o

echo "[2/5] Compiling kernel..."
clang -target i686-elf -Wall -Wextra -nostdlib -ffreestanding -m32 \
    -fno-stack-protector -fno-pie -c kernel.c -o kernel.o

echo "[3/5] Linking kernel..."
# macOS needs different linker flags
clang -target i686-elf -nostdlib -m32 -T linker.ld -o kernel.bin boot.o kernel.o

echo "[4/5] Creating ISO image..."
mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/
cat > isodir/boot/grub/grub.cfg << 'EOF'
menuentry "CustomOS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# Create ISO with grub
if command -v grub-mkrescue &> /dev/null; then
    grub-mkrescue -o customos.iso isodir
    echo "[5/5] ISO created: customos.iso"
elif command -v grub2-mkrescue &> /dev/null; then
    grub2-mkrescue -o customos.iso isodir
    echo "[5/5] ISO created: customos.iso"
else
    echo "[5/5] WARNING: grub-mkrescue not found. Install GRUB."
    exit 1
fi

echo ""
echo "========================================="
echo "  Build complete!"
echo "========================================="
echo ""
echo "To run in QEMU:"
echo "  qemu-system-i386 -cdrom customos.iso"
echo ""
echo "Or with more options:"
echo "  qemu-system-i386 -cdrom customos.iso -m 512M"
echo ""

# Optionally run immediately
read -p "Run CustomOS now in QEMU? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if command -v qemu-system-i386 &> /dev/null; then
        qemu-system-i386 -cdrom customos.iso -m 512M
    else
        echo "QEMU not found. Install with: brew install qemu"
    fi
fi