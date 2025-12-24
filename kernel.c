//custom os start 
//kernal.c 
//author cybermyki


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

// Simple keyboard input (PS/2)
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

// Simple command processor
void process_command(const char* cmd) {
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p') {
        print("\nAvailable commands:\n", VGA_COLOR(YELLOW, BLACK));
        print("  help  - Show this help\n", VGA_COLOR(WHITE, BLACK));
        print("  clear - Clear screen\n", VGA_COLOR(WHITE, BLACK));
        print("  info  - System information\n", VGA_COLOR(WHITE, BLACK));
        print("  time  - Show uptime\n", VGA_COLOR(WHITE, BLACK));
    } else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e') {
        clear_screen();
    } else if (cmd[0] == 'i' && cmd[1] == 'n' && cmd[2] == 'f') {
        print("\nCustomOS v1.0\n", VGA_COLOR(LIGHT_CYAN, BLACK));
        print("Architecture: x86_64\n", VGA_COLOR(WHITE, BLACK));
        print("Built with: Clang 16.0.0\n", VGA_COLOR(WHITE, BLACK));
    } else if (cmd[0] == 't' && cmd[1] == 'i' && cmd[2] == 'm') {
        print("\nUptime: Running...\n", VGA_COLOR(GREEN, BLACK));
    } else if (cmd[0] != 0) {
        print("\nUnknown command. Type 'help' for commands.\n", 
              VGA_COLOR(RED, BLACK));
    }
}

// Main kernel entry point
void kernel_main(void) {
    clear_screen();
    
    print("========================================\n", VGA_COLOR(CYAN, BLACK));
    print("     Welcome to CustomOS v1.0!         \n", VGA_COLOR(LIGHT_CYAN, BLACK));
    print("========================================\n", VGA_COLOR(CYAN, BLACK));
    print("\n", 0);
    print("A simple operating system for learning\n", VGA_COLOR(WHITE, BLACK));
    print("Type 'help' for available commands.\n\n", VGA_COLOR(YELLOW, BLACK));
    
    // Simple shell loop
    char cmd_buffer[256];
    int cmd_index = 0;
    
    print("> ", VGA_COLOR(GREEN, BLACK));
    
    while (1) {
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
        
        // Simple delay to prevent busy waiting
        for (volatile int i = 0; i < 1000; i++);
    }
}