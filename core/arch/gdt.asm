;  gdt.asm
;  
;  Copyright 2013 JS <js@duck-squirell>
;  
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;  
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;  
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
;  MA 02110-1301, USA.
;  
;  


; gdt.asm -- Contains global descriptor table and interrupt descriptor table
;            setup code.

;          Based on code from Bran's kernel development tutorials.
;          Based on code from JamesM's kernel development tutorials.

[GLOBAL gdt_flush]    ; Allows the C code to call gdt_flush().

gdt_flush:
    mov eax, [esp+4]  ; Get the pointer to the GDT, passed as a parameter.
    lgdt [eax]        ; Load the new GDT pointer

    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
    ret

[GLOBAL idt_flush]    ; Allows the C code to call idt_flush().

idt_flush:
    mov eax, [esp+4]  ; Get the pointer to the IDT, passed as a parameter. 
    lidt [eax]        ; Load the IDT pointer.
    ret

[GLOBAL tss_flush]    ; Allows our C code to call tss_flush().
tss_flush:
    mov ax, 0x2B      ; Load the index of our TSS structure - The index is
                      ; 0x28, as it is the 5th selector and each is 8 bytes
                      ; long, but we set the bottom two bits (making 0x2B)
                      ; so that it has an RPL of 3, not zero.
    ltr ax            ; Load 0x2B into the task state register.
    ret
