; Comprehensive test of VM features
; Tests: LOADI, ALU ops, LOAD/STORE, CALL/RET, branches, MMIO, OUT
; Usage:
;   asm examples/comprehensive_test.asm -o build/comprehensive_test.vmb --with-header --entry start
;   vm_app build/comprehensive_test.vmb --verify --dump

start:
        ; Test 1: Basic arithmetic
        LOADI R0, 10
        LOADI R1, 5
        ADD   R2, R0, R1    ; R2 = 15
        SUB   R3, R0, R1    ; R3 = 5
        OUT   R2            ; Print 15
        OUT   R3            ; Print 5
        
        ; Test 2: Memory operations
        LOADI R4, 100       ; Value to store
        LOADI R5, 0x1000    ; Address
        STORE [R5 + 0], R4  ; Store 100 at 0x1000
        LOAD  R6, [R5 + 0]  ; Load back into R6
        OUT   R6            ; Should print 100
        
        ; Test 3: CALL/RET
        LOADI R0, 42
        CALL  print_func
        
        ; Test 4: MMIO console device (at 0xFF00 for 64KB memory)
        LOADI R0, 99
        LOADI R1, 0xFF00
        STORE [R1 + 0], R0  ; Print 99 via MMIO
        
        ; Test 5: Conditional branch
        LOADI R0, 5
        LOADI R1, 5
        CMP   R0, R1        ; Should set Z flag
        JZ    equal_branch
        OUT   R0            ; Should not execute
        JMP   end
        
equal_branch:
        LOADI R7, 777
        OUT   R7            ; Should print 777
        
end:
        HALT

print_func:
        OUT   R0
        RET
