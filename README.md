## What is LogicSim?

LogicSim is a program that simulates a set of rules used to represent logical circuits on a 2d grid. The rules are designed so that the 
2d grid can be interpreted, edited and imported as an image. Using LogicSim, large and complicated logical ciruits can be simulated. 
LogicSim also allows these circuits to interact with the user in real-time and to perform file io via various `Peripheral`s. This allows
one to make complex interactive circuits - including a microprocessor connected to a terminal output that reads a program from a conected
ROM-like module to print "Hi" to the terminal. For this, and other such examples, see the `example\` directory.

## Build instructions:

Building has only been tested with gcc. For gcc, execute the following compile commands in the project root folder, containing sources, 
binaries and includes. For other compilers, the compile statements below can be translated.

```
g++ --std=c++11 -Wall -I includes/ -lncurses -o binaries/logicsim sources/*.cpp includes/*.hpp includes/*.h
g++ -g --std=c++11 -Wall -I includes/ -lncurses -o binaries/logicsim.debug sources/*.cpp includes/*.hpp includes/*.h
```

## What are logical circuits in LogicSim?

In LogicSim, a logical circuit is a n x m grid of logic elements. Logic elements represent the most general possible boolean function of 
four boolean inputs, that is, each logic element is a bool f(bool, bool, bool, bool) and any pure function satisfying the signature can be 
represented with a logic element in LogicSim. Thus, each logic element can be thought of as a chip that takes four digital inputs and produces 
one digital output. At any point of time, each point in the circuit, that is, the position of each logic element, holds a single bit of digital 
data. Each logic element reads 4 bits from the positions surrounding it as input, and puts the output bit in it's own position. LogicSim 
circuits are lossless image files with each pixel representing a logic element. All the information needed to define a logic element is packed 
into the RGB value for the corresponding pixel. See below for details about this packing:

### How are logic elements represented?

Any `bool f(bool, bool, bool, bool)` can be represented by 16 bits, one for each possible combination of inputs(this is the truth table for f). 
If the inputs are `a0`, `a1`, `a2`, and `a3`, we can arrange the input the bits into the 4-bit integer `a = a3a2a1a0` which takes values from 0 to 
15. We can completely and generally define `f` by providing output bits `b0`, `b1`, ..., `b15` for each possible value of `a`. Then, we can represent 
`f` by the 16-bit integer `b15b14b13b12b11b10b9b8b7b6b5b4b3b2b1b0`. Thus, we can in this manner represent any boolean function of 4 variables by a 
16-bit integer. This gives us a way to represent logic elements with a 16-bit integer. The arguemnts to the function is taken according to the following 
diagram:

```
** a1 **
a2 ff a0
** a3 ** 
```

Here, the logic element at the position marked `ff` takes the inputs `a0`, `a1`, `a2` and `a3` from the positions marked as `a0`, `a1`,`a2`, and `a3` 
respectively.

One can represent logic element equivalents for various real life logical circuits, for example one can represent an **AND** gate which produces the 
**AND** of the left and top elements with the 16-bit integer `1000100010001000`, or a **XOR** gate which produces the **XOR** of the left, top and 
bottom elements as `1010010101011010`, and so on. We ca also transfer digital data from one part of the circuit to another part using wires - we define 
wires by placing logic elements that simply copy the bit from one of it's inputs in proper contagious positions. For example, the logic element `gg` 
represented by `1111000011110000` copies the bit at it's immediate right into it's own position. Similarly, the logic element `hh` given by 
`1100110011001100` copies the bit at it's immediate top to it's positon. Then in the following circuit the data at the position marked aa is migrated 
over time to the position marked `bb`. So, we have wires.

```
** ** ** ** ** ** ** **
aa gg gg gg gg gg gg **
** ** ** ** ** ** hh **
** ** ** ** ** ** hh **
** ** ** ** ** ** hh **
** ** ** ** ** ** hh gg=bb
```

But we have a problem. Consider the following senario where we want a wire carrying data downward and a wire carrying data rightward to cross:

```
** ** hh ** **
** ** hh ** **
gg gg xx gg gg
** ** hh ** **
** ** hh ** **
```

What do we put at the position marked `xx`? If we put `gg`, the rightward wire works, but the downward wire is broken. Similarly, putting `hh` 
will not have a desired result. In fact, nomatter what we put in `xx`, it will output a single bit to it's position, and that bit cannot be guaranteed 
to b consistent with both wires. Thus, we have a problem.

In LogicSim we solve this problem by allowing a logic element to read it's input `ai` from not just the position marked `pi`, but also `qi`, as shown 
below. We add 4 extra bits, `c3`, `c2`, `c1`, and `c0` to the beginning of the logic element representation, so that the representation now becomes the 
24-bit intger `c3c2c1c0b16b15b14b13b12b11b10b9b8b7b6b5b4b3b2b1b0`. If `ci` is 0, `ai` is taken from `pi`, else it's taken from `qi`.

```
** ** q1 ** **
** ** p1 ** **
q2 p2 ff p0 q0
** ** p3 ** **
** ** q3 ** **
```

Now, we solve the intersecting wires problem by defining the logic element `kk` which copies the bit from 2 cells above it as `00101100110011001100`. 
Then the intersecting wires circuit can be redesigned as:

```
** ** hh ** **
** ** hh ** **
gg gg gg gg gg
** ** kk ** **
** ** hh ** **
```

Here  `kk` basically defines a skip, or jump element. With the logic elements defined thus, we can create large and complicated logical circuits, like 
binary adders, multipliers, multiplexers, registers, control units, or even programmable processors.

We now pack the representation of the logic element into RGB values as follows:

```
R=****c3c2c1c0
G=b15b14b13b12b11b10b9b8
B=b7b6b5b4b3b2b1b0
```

Here, the bits represented by `*` can have any value, they do not affect the representation of the logic element. They can be set to convinient values 
for visual aid while editing the circuit diagram as an image.

## What are peripherals in LogicSim?

In LogicSim, a peripheral is simply anything that reads bits of data from and writes bits of data to the circuit board in realtime. They are used to 
represent interfacing peripherals for the circuit. For example, a peripheral could read certain bits from the circuit board and print the value of these 
bits to the terminal in realtime. If a bit is written to by a peripheral and a logic element, the peripheral always gets precedence. However, if multiple 
peripherals write to the same bit, undefined behavior ensues. This situation should generally be avoided. For a description of available peripherals, see 
peripherals.hpp.

## The File Format for LogicSim Systems:

A LogicSim *System* cosists of a LogicSim circuit, and optionally some peripherals. A LogicSim system is specified by a JSON file. The top level object 
of the JSON file has 2 fields, `"Image path"` whose corresponding value should be a string which provides a path to a lossless image file (preferably of 
.png format) containing the circuit diagram, and `"Peripherals"` whose value is an array of JSON objects, one representing each peripheral attached. The 
JSON object representing a peripheral itself has two fields, `"Class"`, whose value is the name of the peripheral class to use for that particular 
peripheral, and `"Initializer"`, whose value is a JSON object whose syntax is specific to the peripheral class and is described in peripherals.hpp.

LogicSim uses non-negative integer coordinates for all positions on the circuit board. The origin is on the top left corner. The X-axix increases to the 
right and the Y-axis increases downwards.
