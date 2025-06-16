; Print a constant number and halt
; Usage:
;   asm examples/print_number.asm -o build/print_number.bin
;   vm_app build/print_number.bin --dump

        LOADI R0, 12345
        OUT   R0
        HALT
