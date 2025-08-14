section .data
newline db 10
str0 db 'even'
str0_len equ $-str0
str1 db 'odd'
str1_len equ $-str1
section .bss
numbuf resb 64
var0 resq 1
section .text
global _start
_start:
    mov rax, 1
    mov [var0], rax
    mov rax, 10
    mov r10, rax
    cmp r10, 0
    jle .L2
.L1:
    mov rax, [var0]
    push rax
    mov rax, 2
    mov rbx, rax
    pop rcx
    mov rax, rcx
    cqo
    idiv rbx
    mov rax, rdx
    push rax
    mov rax, 0
    mov rbx, rax
    pop rcx
    cmp rcx, rbx
    mov rax, 0
    sete al
    cmp rax, 0
    je .L3
    mov rax, 1
    mov rdi, 1
    mov rsi, str0
    mov rdx, str0_len
    syscall
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall
    jmp .L4
.L3:
    mov rax, 1
    mov rdi, 1
    mov rsi, str1
    mov rdx, str1_len
    syscall
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall
.L4:
    mov rax, [var0]
    push rax
    mov rax, 1
    mov rbx, rax
    pop rcx
    add rcx, rbx
    mov rax, rcx
    mov [var0], rax
    dec r10
    jnz .L1
.L2:
    mov rax, 255
    mov rdi, rax
    mov rax, 60
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
