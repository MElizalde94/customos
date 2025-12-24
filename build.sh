#!/bin/bash
echo "========================================="
echo "  Building CustomOS v1.0"
echo "========================================="

rm -f *.o kernel.bin customos.iso
rm -rf isodir

echo "[1/5] Compiling bootloader..."
nasm -f elf32 boot.asm -o boot.o

echo "[2/5] Compiling kernel..."
clang -target i686-elf -Wall -Wextra -nostdlib -ffreestanding -m32 \
    -fno-stack-protector -fno-pie -c kernel.c -o kernel.o

echo "[3/5] Linking kernel..."
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o

echo "[4/5] Creating ISO image..."
mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/
cat > isodir/boot/grub/grub.cfg << 'GRUBEOF'
menuentry "CustomOS" {
    multiboot /boot/kernel.bin
    boot
}
GRUBEOF

if command -v grub-mkrescue &> /dev/null; then
    grub-mkrescue -o customos.iso isodir
    echo "[5/5] ISO created: customos.iso"
else
    echo "[5/5] WARNING: grub-mkrescue not found."
    exit 1
fi

echo ""
echo "Build complete! Run with:"
echo "  qemu-system-i386 -cdrom customos.iso"