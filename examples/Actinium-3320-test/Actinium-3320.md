A brief documentation / reference for various instruction sets/ op-codes related to Actininum.

### Binary Operations Unit Selection Line Codes

**Code**    |   **Function**
:----------:|:---------------:
001         |   And
010         |   Or
011         |   Not
100         |   Add
110         |   Shift Left (Multiply by 2)
111         |   Shift Right

### Addressing codes for registers and IO

    The Actinium processor has one common addressing space which is used for registers and 
IO ports. IO ports are arranged in pairs of associated input and output ports, both of which 
share the same address. Since both read and write operations to this addressing space never
occurs in the same instruction, this is not a problem. Note that the processor has an extra
register - the accumulator - which does not belong to any address space.

**Code**    |   **Address**
:----------:|:------------:
000         |   Register 0
001         |   Register 1
010         |   Register 2
011         |   Register 3
100         |   Register 4
101         |   IO port 1
110         |   IO port 2
111         |   IO port 3

### Instructions:

    Instructions have 2 parts, the first 5 most significant bits form the opcode, and the 6th, 
7th and 8th bits form the operator. When the operand is interpreted as a constant, the constant
is constructed by setting the lest significant 3 bits to the operand's value, and leaving the rest
of the bits to 0. Note that using op-codes other than the ones mentioned here produces udefined
behavior. The opcodes and a brief description of the behavior of each instruction is given below, 
where `ACC` refers to the accumulator:

**Opcode**  |                                       **Description**                                             |
:----------:|:-------------------------------------------------------------------------------------------------:|
00000       | Increment opcode address without making any other changes to registers, outputs or `ACC`.         |
00010       | Reads data from operand's location, bitwise ANDs it with `ACC`, and stores in `ACC`.              |
00011       | Interprets operand as constant, bitwise ANDs it with `ACC`, and stores result in `ACC`.           |
00100       | Reads data from operand's location, bitwise ORs it with `ACC`, and stores in `ACC`.               |
00101       | Interprets operand as constant, bitwise ORs it with `ACC`, and stores result in `ACC`.            |
00110       | Flips every bit in ACC                                                                            |
01000       | Reads data from operand's location, ADDs it to `ACC` as unsignned int, and stores in `ACC`.       |
01001       | Interprets operand as constant, ADDs it to `ACC`, and stores result in `ACC`.                     |
01100       | Shifts value of `ACC` to more significant side by as many places as value located at the operand. |
01110       | Shifts value of `ACC` to less significant side by as many places as value located at the operand. |
01101       | Shifts value of `ACC` to more significant side by as many places as value of the operand.         |
01111       | Shifts value of `ACC` to less significant side by as many places as value of the operand.         |
11000       | Reads data from operand's location and copies it to `ACC`.                                        |
11001       | Interprets operand as constant and copies it to `ACC`.                                            |
11100       | Copies `ACC` to location of operand.                                                              |
10110       | Increases instruction address by value of operand, ie, jumps forward by value of operand.         |
10111       | Jumps backward by value of operand.                                                               |
10100       | If the most significant bit of ACC is 1, jumps forward by value of operand, else moves to next.   |
10101       | If the most significant bit of ACC is 0, jumps backward by value of operand, else moves to next.  |

### Timings:

The timing unit counts through 8 stages, each stage lasts atleasts 50 ticks. The stages are described:

0. The zeroth stage holds the instruction read gate high.
1. By the end of first stage, the instruction address output is ready.
2. The second stage holds the register and IO write line high, as register-IO addressing is complete by this point.
   The instruction address loopback gate 1 write is also held high to produce instruction address output. 
3. The third stage does nothing, all lines are held low.
4. The fourth stage holds the instruction address loopback gate 2 write line high, instruction address loopback is complete by
   this point.
5. The fifth stage holds the ACC1 register write line high.
6. The sixth stage does nothing.
7. The seventh stage holds the ACC2 register write line high.

Thus, the processor must be fed a clock signal of time period of 50 ticks, which corresponds to a clock frequency of 20 KHz under
the microsecond rule. Each instruction takes 8 clock pulses to b processed, thus taking 400 ticks, producing an instruction
frequency of 2.5 KHz.
