__BOOT_CS equ 0x10

__BOOT_DS equ 0x18

section .text

global ReloadSegments

ReloadSegments
   push 0x10
   lea rax, [rel reload_cs] ; Load address of .reload_CS into RAX
   push rax                  ; Push this value to the stack
   retfq
   reload_cs:
   ; Reload data segment registers
   mov   ax, 0x18 ; 0x18 is a stand-in for your data segment
   mov   ds, ax
   mov   es, ax
   mov   ss, ax
   ret