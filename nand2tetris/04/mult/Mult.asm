// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.

// Compute var a times var b
@0
D=M
@a
M=D

@1
D=M
@b
M=D

@result
M=0

(LOOP)
// If var a == 0; then exit LOOP
@a
D=M
@END
D; JEQ

@b
D=M
@result
M=M + D
@a
M=M - 1

// goto loop beginning
@LOOP
0;JEQ

(END)

@result
D=M
@2
M=D

(EXIT)
@EXIT
0;JEQ
