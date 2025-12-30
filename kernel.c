//custom os start 
//kernal.c 
//author cybermyki
// Compile: clang -Wall -Wextra -nostdlib -ffreestanding -c kernel.c -o kernel.o

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

// VGA text mode buffer
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
static uint32_t vga_index = 0;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR(fg, bg) ((bg << 4) | fg)

enum vga_color {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
    RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GRAY = 7,
    DARK_GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
    LIGHT_CYAN = 11, LIGHT_RED = 12, LIGHT_MAGENTA = 13,
    YELLOW = 14, WHITE = 15
};

// System uptime counter
static uint32_t uptime_seconds = 0;
static uint32_t tick_count = 0;

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (VGA_COLOR(LIGHT_GRAY, BLACK) << 8) | ' ';
    }
    vga_index = 0;
}

void print_char(char c, uint8_t color) {
    if (c == '\n') {
        vga_index = (vga_index / VGA_WIDTH + 1) * VGA_WIDTH;
    } else {
        vga_buffer[vga_index] = (color << 8) | c;
        vga_index++;
    }
    
    if (vga_index >= VGA_WIDTH * VGA_HEIGHT) {
        vga_index = 0;
    }
}

void print(const char* str, uint8_t color) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i], color);
    }
}

// Helper functions for number printing
void print_hex(uint32_t num, uint8_t color) {
    char hex[] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        uint8_t digit = num & 0xF;
        hex[i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
        num >>= 4;
    }
    print(hex, color);
}

void print_dec(uint32_t num, uint8_t color) {
    if (num == 0) {
        print_char('0', color);
        return;
    }
    
    char buffer[12];
    int i = 0;
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        print_char(buffer[j], color);
    }
}

// Keyboard and port I/O
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

char scancode_to_char(uint8_t scancode) {
    static const char scancode_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_map)) {
        return scancode_map[scancode];
    }
    return 0;
}

// RTC (Real-Time Clock) functions
#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

uint8_t read_cmos(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

void read_rtc(uint8_t* hours, uint8_t* minutes, uint8_t* seconds) {
    *seconds = bcd_to_binary(read_cmos(0x00));
    *minutes = bcd_to_binary(read_cmos(0x02));
    *hours = bcd_to_binary(read_cmos(0x04));
}

void show_clock() {
    uint8_t hours, minutes, seconds;
    read_rtc(&hours, &minutes, &seconds);
    
    print("\n=== Real-Time Clock ===\n", VGA_COLOR(CYAN, BLACK));
    print("Current Time: ", VGA_COLOR(WHITE, BLACK));
    
    if (hours < 10) print_char('0', VGA_COLOR(GREEN, BLACK));
    print_dec(hours, VGA_COLOR(GREEN, BLACK));
    print_char(':', VGA_COLOR(WHITE, BLACK));
    
    if (minutes < 10) print_char('0', VGA_COLOR(GREEN, BLACK));
    print_dec(minutes, VGA_COLOR(GREEN, BLACK));
    print_char(':', VGA_COLOR(WHITE, BLACK));
    
    if (seconds < 10) print_char('0', VGA_COLOR(GREEN, BLACK));
    print_dec(seconds, VGA_COLOR(GREEN, BLACK));
    print("\n", 0);
    
    print("\nSystem Uptime: ", VGA_COLOR(WHITE, BLACK));
    uint32_t hours_up = uptime_seconds / 3600;
    uint32_t mins_up = (uptime_seconds % 3600) / 60;
    uint32_t secs_up = uptime_seconds % 60;
    
    print_dec(hours_up, VGA_COLOR(YELLOW, BLACK));
    print("h ", VGA_COLOR(WHITE, BLACK));
    print_dec(mins_up, VGA_COLOR(YELLOW, BLACK));
    print("m ", VGA_COLOR(WHITE, BLACK));
    print_dec(secs_up, VGA_COLOR(YELLOW, BLACK));
    print("s\n", VGA_COLOR(WHITE, BLACK));
}

// Memory viewer
void view_memory(uint32_t address, uint32_t length) {
    print("\n=== Memory Viewer ===\n", VGA_COLOR(CYAN, BLACK));
    print("Address: ", VGA_COLOR(WHITE, BLACK));
    print_hex(address, VGA_COLOR(YELLOW, BLACK));
    print(" | Length: ", VGA_COLOR(WHITE, BLACK));
    print_dec(length, VGA_COLOR(YELLOW, BLACK));
    print(" bytes\n\n", VGA_COLOR(WHITE, BLACK));
    
    uint8_t* ptr = (uint8_t*)address;
    
    for (uint32_t i = 0; i < length; i += 16) {
        // Print address
        print_hex(address + i, VGA_COLOR(DARK_GRAY, BLACK));
        print(":  ", VGA_COLOR(WHITE, BLACK));
        
        // Print hex values
        for (int j = 0; j < 16 && (i + j) < length; j++) {
            uint8_t byte = ptr[i + j];
            char hex[3];
            hex[0] = (byte >> 4) < 10 ? '0' + (byte >> 4) : 'A' + (byte >> 4) - 10;
            hex[1] = (byte & 0xF) < 10 ? '0' + (byte & 0xF) : 'A' + (byte & 0xF) - 10;
            hex[2] = '\0';
            print(hex, VGA_COLOR(LIGHT_GREEN, BLACK));
            print_char(' ', VGA_COLOR(WHITE, BLACK));
            
            if (j == 7) print_char(' ', VGA_COLOR(WHITE, BLACK));
        }
        
        // Padding for incomplete lines
        for (int j = (length - i > 16 ? 16 : length - i); j < 16; j++) {
            print("   ", VGA_COLOR(WHITE, BLACK));
            if (j == 7) print_char(' ', VGA_COLOR(WHITE, BLACK));
        }
        
        // Print ASCII representation
        print(" |", VGA_COLOR(DARK_GRAY, BLACK));
        for (int j = 0; j < 16 && (i + j) < length; j++) {
            uint8_t byte = ptr[i + j];
            char c = (byte >= 32 && byte <= 126) ? byte : '.';
            print_char(c, VGA_COLOR(CYAN, BLACK));
        }
        print("|\n", VGA_COLOR(DARK_GRAY, BLACK));
        
        // Limit output to prevent screen overflow
        if (i >= 160) {
            print("\n[Output truncated - use smaller length]\n", VGA_COLOR(RED, BLACK));
            break;
        }
    }
}

// String comparison helper
int str_starts_with(const char* str, const char* prefix) {
    for (int i = 0; prefix[i] != '\0'; i++) {
        if (str[i] != prefix[i]) return 0;
    }
    return 1;
}

// Parse hex number from string
uint32_t parse_hex(const char* str) {
    uint32_t result = 0;
    int i = 0;
    
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) i = 2;
    
    for (; str[i] != '\0' && str[i] != ' '; i++) {
        result *= 16;
        if (str[i] >= '0' && str[i] <= '9') {
            result += str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            result += str[i] - 'a' + 10;
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            result += str[i] - 'A' + 10;
        }
    }
    return result;
}

// Command processor
void process_command(const char* cmd) {
    if (str_starts_with(cmd, "help")) {
        print("\nAvailable commands:\n", VGA_COLOR(YELLOW, BLACK));
        print("  help       - Show this help\n", VGA_COLOR(WHITE, BLACK));
        print("  clear      - Clear screen\n", VGA_COLOR(WHITE, BLACK));
        print("  info       - System information\n", VGA_COLOR(WHITE, BLACK));
        print("  clock      - Show real-time clock\n", VGA_COLOR(WHITE, BLACK));
        print("  time       - Show uptime\n", VGA_COLOR(WHITE, BLACK));
        print("  mem [addr] - View memory at address (hex)\n", VGA_COLOR(WHITE, BLACK));
        print("               Example: mem 0xB8000\n", VGA_COLOR(DARK_GRAY, BLACK));
        print("  meminfo    - Show memory statistics\n", VGA_COLOR(WHITE, BLACK));
    } else if (str_starts_with(cmd, "clear")) {
        clear_screen();
    } else if (str_starts_with(cmd, "info")) {
        print("\nCustomOS v1.0 - Enhanced Edition\n", VGA_COLOR(LIGHT_CYAN, BLACK));
        print("Architecture: x86 (i386)\n", VGA_COLOR(WHITE, BLACK));
        print("Built with: Clang 16.0.0\n", VGA_COLOR(WHITE, BLACK));
        print("Features: RTC, Memory Viewer\n", VGA_COLOR(GREEN, BLACK));
    } else if (str_starts_with(cmd, "clock")) {
        show_clock();
    } else if (str_starts_with(cmd, "time")) {
        show_clock();
    } else if (str_starts_with(cmd, "meminfo")) {
        print("\n=== Memory Information ===\n", VGA_COLOR(CYAN, BLACK));
        print("VGA Buffer:    ", VGA_COLOR(WHITE, BLACK));
        print_hex(0xB8000, VGA_COLOR(YELLOW, BLACK));
        print("\n", 0);
        print("Kernel Start:  ", VGA_COLOR(WHITE, BLACK));
        print_hex(0x100000, VGA_COLOR(YELLOW, BLACK));
        print(" (1 MB)\n", 0);
        print("Stack Top:     ", VGA_COLOR(WHITE, BLACK));
        print_hex((uint32_t)&vga_buffer, VGA_COLOR(YELLOW, BLACK));
        print("\n", 0);
    } else if (str_starts_with(cmd, "mem")) {
        // Parse address from command
        const char* addr_str = cmd + 3;
        while (*addr_str == ' ') addr_str++; // Skip spaces
        
        if (*addr_str == '\0') {
            // Default: view VGA buffer
            view_memory(0xB8000, 128);
        } else {
            uint32_t addr = parse_hex(addr_str);
            view_memory(addr, 128);
        }
    } else if (cmd[0] != 0) {
        print("\nUnknown command: '", VGA_COLOR(RED, BLACK));
        print(cmd, VGA_COLOR(RED, BLACK));
        print("'\nType 'help' for available commands.\n", VGA_COLOR(RED, BLACK));
    }
}

// Main kernel entry point
void kernel_main(void) {
    clear_screen();
    
    print("========================================\n", VGA_COLOR(CYAN, BLACK));
    print("   Welcome to CustomOS v1.0 Enhanced!  \n", VGA_COLOR(LIGHT_CYAN, BLACK));
    print("========================================\n", VGA_COLOR(CYAN, BLACK));
    print("\n", 0);
    print("Features: Real-Time Clock, Memory Viewer\n", VGA_COLOR(GREEN, BLACK));
    print("Type 'help' for available commands.\n\n", VGA_COLOR(YELLOW, BLACK));
    
    // Simple shell loop
    char cmd_buffer[256];
    int cmd_index = 0;
    
    print("> ", VGA_COLOR(GREEN, BLACK));
    
    while (1) {
        // Update uptime (simple tick counter)
        tick_count++;
        if (tick_count >= 100000) {
            tick_count = 0;
            uptime_seconds++;
        }
        
        // Check if keyboard data is available
        uint8_t status = inb(KEYBOARD_STATUS_PORT);
        if (status & 0x01) {
            uint8_t scancode = inb(KEYBOARD_DATA_PORT);
            
            // Only process key press (not release)
            if (!(scancode & 0x80)) {
                char c = scancode_to_char(scancode);
                
                if (c == '\n') {
                    cmd_buffer[cmd_index] = '\0';
                    print_char('\n', VGA_COLOR(WHITE, BLACK));
                    process_command(cmd_buffer);
                    cmd_index = 0;
                    print("\n> ", VGA_COLOR(GREEN, BLACK));
                } else if (c != 0 && cmd_index < 255) {
                    cmd_buffer[cmd_index++] = c;
                    print_char(c, VGA_COLOR(WHITE, BLACK));
                }
            }
        }
        
        // Simple delay
        for (volatile int i = 0; i < 1000; i++);
    }
}