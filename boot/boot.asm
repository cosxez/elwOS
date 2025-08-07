bits 32
section .multiboot
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .text
global start
extern kernel_main
extern keyboard

setup_idt:
    ; Заполняем IDT нулями
    mov edi, IDT
    mov ecx, 256
    xor eax, eax
    rep stosd

    ; Настраиваем обработчик клавиатуры (IRQ1 -> INT 0x21)
    mov eax, irq1
    mov word [IDT + 0x21*8], ax       ; Младшие 16 бит адреса
    mov word [IDT + 0x21*8 + 2], 0x08 ; Селектор
    mov word [IDT + 0x21*8 + 4], 0x8E00 ; Атрибуты
    shr eax, 16
    mov word [IDT + 0x21*8 + 6], ax   ; Старшие 16 бит адреса

    lidt [idt_descriptor]
    ret


start:
    mov esp, 0x7FFFF
    call setup_idt
    call kernel_main
    hlt

global irq1
irq1:
    pusha
    call keyboard
    ; Отправляем EOI
    mov al, 0x20
    out 0x20, al
    popa
    iret


section .data
idt_descriptor:
    dw 256*8 - 1
    dd IDT
IDT:
    times 256 dq 0

