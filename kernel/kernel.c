volatile unsigned short *vga_buffer = (unsigned short*)0xB8000;

// Вывод символа на экран
void vga_putchar(char c) {
    static int cursor_x = 0, cursor_y = 0;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        vga_buffer[cursor_y * 80 + cursor_x] = 0x0F00 | c;
        cursor_x++;
    }
    
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Прокрутка экрана
    if (cursor_y >= 25) {
        for (int i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        for (int i = 24 * 80; i < 25 * 80; i++) {
            vga_buffer[i] = 0x0F00 | ' ';
        }
        cursor_y = 24;
    }
}

extern void idt_init();

void kernel_main() {
    // Очистка экрана
    volatile unsigned short *vga = (unsigned short*)0xB8000;
    for (int i = 0; i < 80*25; i++) 
        vga[i] = 0x0F00 | ' ';
   


    



    // Инициализация ввода
    idt_init();
    asm volatile("sti");  // Разрешаем прерывания
    
    // Приветственное сообщение
    const char *hello = "This is elwOS. Write \"help\" for watching commands.";

    for (int p = 0; hello[p] != '\0'; p++)
        vga[p] = 0x0F00 | hello[p];


    while(1) asm("hlt");  // Экономия энергии
}
