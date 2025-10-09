#include <stdint.h>
#include "io.c"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void main() {
    unsigned short *vram = (unsigned short*)0xb8000;
    const unsigned char color = 7; // gray on black
    int cursor = 0;

    while(1) {
        uint8_t status = inb(0x64);
        if(status & 1) {
            uint8_t scancode = inb(0x60);

            // Print scancode as two hex chars for visibility
            vram[cursor++] = (color << 8) | "0123456789ABCDEF"[scancode >> 4];
            vram[cursor++] = (color << 8) | "0123456789ABCDEF"[scancode & 0xF];
            vram[cursor++] = (color << 8) | ' ';

            // Wrap around at bottom of screen
            if(cursor >= 80 * 25) cursor = 0;
        }
    }
}
