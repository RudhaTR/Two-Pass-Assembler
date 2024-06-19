; Initialize the stack pointer
ldc 0x1000
a2sp

; Load the address of the first number
ldc numbers
stl 0 ; Store the address in memory at SP + 0

; Initialize the sum to 0
ldc 0
stl 1 ; Store the sum (0) in memory at SP + 1

; Loop to add the numbers
loop:
ldl 0 ; Load the address of the current number into the accumulator
ldnl 0 ; Load the value at the current address into the accumulator
ldl 1 ; Load the current sum into the accumulator
add   ; Add the current number to the sum
stl 1 ; Store the new sum in memory at SP + 1
ldc 1 ; Load the constant 1 into the accumulator
ldl 0 ; Load the current address into the accumulator
add   ; Increment the address to point to the next number
stl 0 ; Store the new address in memory at SP + 0
ldl 0 ; Load the new address into the accumulator
ldnl 0 ; Load the value at the new address into the accumulator
brz done ; If the value is 0 (end of the list), branch to done
br loop ; Otherwise, continue the loop

done:
ldl 1 ; Load the final sum into the accumulator
ldc 0;
stnl 0;

HALT ; Stop the program execution

; Data section
numbers:
data 1
data 2
data 3
data 4
data 5
data 0 ; Sentinel value to mark the end of the list