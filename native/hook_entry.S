/* Win64 entry shim: preserve the ordinary caller's R12 plot row before C can
 * reuse that nonvolatile register. Copy stack arguments into a fresh call frame.
 * Unwind metadata describes the sole stack allocation. No game/Lua calls here. */
.intel_syntax noprefix
.text
.globl bridge_net_entry
.def bridge_net_entry; .scl 2; .type 32; .endef
.seh_proc bridge_net_entry
bridge_net_entry:
    sub rsp, 0x58
    .seh_stackalloc 0x58
    .seh_endprologue
    mov rax, [rsp + 0x80]
    mov [rsp + 0x20], rax
    mov rax, [rsp + 0x88]
    mov [rsp + 0x28], rax
    mov rax, [rsp + 0x90]
    mov [rsp + 0x30], rax
    mov [rsp + 0x38], r12
    mov rax, [rsp + 0x58]
    mov [rsp + 0x40], rax
    call bridge_net
    add rsp, 0x58
    ret
.seh_endproc
