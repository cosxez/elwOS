#include <stdint.h>

#define PORT_KEYBOARD_DATA 0x60
#define PORT_KEYBOARD_CTRL 0x64
#define PIC1_CMD 0x20

extern volatile unsigned short *vga_buffer;
static int cursor_x = 0, cursor_y = 0;

// Карта скан-кодов (US QWERTY)
static const char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

// Ассемблерные инструкции
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Обработчик прерывания (ассемблерная обёртка)
__attribute__((naked)) static void keyboard_handler_asm() {
    asm volatile(
        "pusha\n\t"
        "call keyboard_handler\n\t"
        "mov $0x20, %al\n\t"
        "out %al, $0x20\n\t"
        "popa\n\t"
        "iret"
    );
}

// Основной обработчик
void keyboard_handler() {
    uint8_t scancode = inb(PORT_KEYBOARD_DATA);
    
    if (scancode < sizeof(keymap) && keymap[scancode]) {
        char c = keymap[scancode];
        if (c == '\b' && cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * 80 + cursor_x] = 0x0F00 | ' ';
        } else if (c >= ' ') {
            vga_buffer[cursor_y * 80 + cursor_x] = 0x0F00 | c;
            cursor_x++;
        }
        
        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
        
        if (cursor_y >= 25) {
            cursor_y = 24;
            for (int i = 0; i < 24*80; i++) 
                vga_buffer[i] = vga_buffer[i+80];
            for (int i = 24*80; i < 25*80; i++)
                vga_buffer[i] = 0x0F00 | ' ';
        }
    }
}

// Инициализация IDT
struct IDT_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct IDT_entry IDT[256];
static struct { uint16_t limit; uint32_t base; } __attribute__((packed)) idtr;

void idt_init() {
    // Настройка обработчика клавиатуры (IRQ1 -> INT 33)
    IDT[33] = (struct IDT_entry){
        .offset_low = (uint32_t)keyboard_handler_asm & 0xFFFF,
        .selector = 0x08,
        .type_attr = 0x8E,
        .offset_high = (uint32_t)keyboard_handler_asm >> 16
    };
    
    // Разрешаем только прерывание клавиатуры
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
    
    // Загружаем IDT
    idtr.limit = sizeof(IDT) - 1;
    idtr.base = (uint32_t)&IDT;
    asm volatile("lidt %0" : : "m"(idtr));
}
