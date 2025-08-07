#include <stdint.h>
#include <string.h>


#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE 0x0F
#define GREEN 0x0A
#define RED 0x04


#define KEYBOARD_PORT 0x60


void print(const char* str, uint8_t color);
void execute_comm(const char* comm);

volatile uint16_t *vga_buff=(uint16_t*)0xB8000;

int cur_x=0;
int cur_y=0;







int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}




size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}





static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
	return ret;
}


static inline void outb(uint16_t port, uint8_t val)
{
	asm volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port));
}


static const char keyboard_map[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
};



char input_buff[256];
uint8_t input_pos=0;






void init_keyboard() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}




void keyboard() {
    uint8_t scancode = inb(KEYBOARD_PORT);
    if (scancode < sizeof(keyboard_map)) {
        char c = keyboard_map[scancode];
        if (c != 0) {
            if (c == '\n') {
                print("\n", WHITE);
                execute_comm(input_buff);
                input_pos = 0;
                input_buff[0] = '\0';
            } else if (c == '\b') {
                if (input_pos > 0) {
                    input_pos--;
                    input_buff[input_pos] = '\0';
                    print("\b \b", WHITE);
                }
            } else {
                print(&c, WHITE);
                input_buff[input_pos] = c;
                input_pos++;
            }
        }
    }
}




void clear_scr()
{
	for (int i=0;i<VGA_HEIGHT;i++)
	{
		for (int u=0; u<VGA_WIDTH;u++)
		{
			vga_buff[i*VGA_WIDTH+u] = (0x00<<8) | ' ';
		}
	}
}


void write(const char text, uint8_t color)
{
	if (text=='\n')
	{
		cur_x=0;
		cur_y+=1;
	}

	else
	{
		vga_buff[cur_y*VGA_WIDTH+cur_x]=(color<<8) | text;
		cur_x+=1;
		if (cur_x>=VGA_WIDTH)
		{
			cur_x=0;
			cur_y+=1;
		}
	}
}


void print(const char* str, uint8_t color)
{
	for (int i=0;str[i]!='\0';i++)
	{
		write(str[i],color);
	}
}



void execute_comm(const char* comm)
{
        if (strcmp(comm,"help")==0)
        {
                print("All commands:\n",WHITE);
                print("    clear - clear screen\n", WHITE);

        }

        else if (strcmp(comm,"clear")==0)
        {
                clear_scr();
                cur_x=0;
                cur_y=0;
        }



        else
        {
                print("Not found command: ", RED);
                print(comm,WHITE);
                print("\n",WHITE);
        }
}



void kernel_main()
{
	init_keyboard();
	asm volatile("sti");
	print("Kernel load\n", WHITE);
	print("Wait...", RED);
	print("Done\n",GREEN);
	print("Welcome to elwOS", WHITE);



	while (1)
	{
		asm volatile("hlt");
	}
}
