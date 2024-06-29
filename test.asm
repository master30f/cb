format ELF64 executable
entry start

segment readable executable

start:
    mov dword [rsp-0x04], -1
    mov edi, [rsp-0x04]
    mov eax, 0x3C
    syscall