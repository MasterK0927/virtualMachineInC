; Demonstrates CALL/RET and labels with header entry
; Usage:
;   asm examples/call_and_ret.asm -o build/call_and_ret.vmb --with-header --entry start
;   vm_app build/call_and_ret.vmb --dump

start:
        LOADI R0, 5      ; value to print
        CALL print
        HALT

print:
        OUT   R0
        RET
