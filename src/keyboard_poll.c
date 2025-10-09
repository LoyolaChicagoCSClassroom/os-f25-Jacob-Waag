#define PS2_STATUS_PORT 0x64
#define PS2_DATA_PORT   0x60

// Returns nonzero if output buffer is full (scancode ready)
int is_output_buffer_full() {
    return inb(PS2_STATUS_PORT) & 0x01;
}

// Read scancode from PS/2 data port
uint8_t read_scancode() {
    return inb(PS2_DATA_PORT);
}
