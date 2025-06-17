; Demonstrates CMP/JZ/JNZ and a simple countdown loop
; Usage:
;   asm examples/branch_and_loop.asm -o build/loop.bin
;   vm_app build/loop.bin --dump

        LOADI R0, 5      ; counter
loop:
        OUT   R0         ; print current value
        LOADI R1, 1
        SUB   R0, R0, R1 ; R0 = R0 - 1
        CMP   R0, R1     ; Z=1 if R0 == 1
        JZ    end
        JNZ   loop
end:
        HALT
