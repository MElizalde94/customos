#!/bin/bash
set -e

echo "========================================="
echo "  Building CustomOS v1.0"
echo "========================================="

rm -f *.o kernel.bin customos.iso
rm -rf isodir

echo "[1/5] Compiling bootloader..."
nasm -f elf32 boot.asm -o boot.o

echo "[2/5] Compiling kernel..."
gcc -m32 -Wall -Wextra -nostdlib -ffreestanding \
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

echo "[5/5] Building ISO with GRUB..."
grub-mkrescue -o customos.iso isodir 2>/dev/null

echo ""
echo "========================================="
echo "  ✓ Build Complete!"
echo "========================================="
echo ""
echo "To run: qemu-system-i386 -cdrom customos.iso -m 512M"
