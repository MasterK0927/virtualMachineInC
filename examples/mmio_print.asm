; Demonstrates memory-mapped I/O to console device
; Console device is mapped at (memSize - 256) = 0xFF00 for 64KB memory
; Usage:
;   asm examples/mmio_print.asm -o build/mmio_print.bin
;   vm_app build/mmio_print.bin --dump

        LOADI R0, 42         ; value to print via MMIO
        LOADI R1, 0xFF00     ; console device base address
        STORE [R1 + 0], R0   ; write to device (prints 42)
        
        LOADI R0, 100        ; another value
        OUT   R0             ; print via OUT opcode (prints 100)
        
        HALT
