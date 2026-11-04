; Win64 equivalent of hook_entry.S. Keep both shims in sync.
EXTERN bridge_net:PROC
PUBLIC bridge_net_entry
.code
bridge_net_entry PROC FRAME
    sub rsp, 58h
    .allocstack 58h
    .endprolog
    mov rax, [rsp + 80h]
    mov [rsp + 20h], rax
    mov rax, [rsp + 88h]
    mov [rsp + 28h], rax
    mov rax, [rsp + 90h]
    mov [rsp + 30h], rax
    mov [rsp + 38h], r12
    mov rax, [rsp + 58h]
    mov [rsp + 40h], rax
    call bridge_net
    add rsp, 58h
    ret
bridge_net_entry ENDP
END
