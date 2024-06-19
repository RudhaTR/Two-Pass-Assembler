; Initialize the stack pointer
ldc 0x1000
a2sp

; Load the first number (6)
ldc num1
ldnl 0  ; Load the value at the address num1 into the accumulator
stl 0   ; Store the first number (6) in memory at SP + 0

; Load the second number (8)
ldc num2
ldnl 0  ; Load the value at the address num2 into the accumulator
stl 1   ; Store the second number (8) in memory at SP + 1

; Initialize the result to 0
ldc 0
stl 2   ; Store the initial result (0) in memory at SP + 2

; Initialize the counter to 0
ldc 0
stl 3   ; Store the initial counter (0) in memory at SP + 3

; Loop to multiply by consecutive addition
loop:
ldl 3   ; Load the counter into the accumulator
ldl 1   ; Load the second number (8) into the accumulator
sub     ; Subtract the counter from the second number
brz done ; If the result is 0, branch to done
ldl 0   ; Load the first number (6) into the accumulator
ldl 2   ; Load the current result into the accumulator
add     ; Add the first number to the current result
stl 2   ; Store the new result in memory at SP + 2
ldc 1   ; Load the constant 1 into the accumulator
ldl 3   ; Load the counter into the accumulator
add     ; Increment the counter by 1
stl 3   ; Store the incremented counter in memory at SP + 3
br loop ; Continue the loop

done:
ldl 2   ; Load the final result into the accumulator
ldc 0;
stnl 0 ; Store the final result in the very first memory location

HALT ; Stop the program execution

; Data section
num1: data 6
num2: data 8