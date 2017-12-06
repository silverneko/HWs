// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

(LOOP_START)

// check if keyboard is pressed
(IF_START)
  @KBD
  D=M
  @KEY_NOT_PRESSED
  D;JEQ
(THEN)
  // keyboard pressed, blacken the screen
  @color
  M=-1
  @IF_END
  0;JMP
(ELSE)
  // keyboard not pressed, whiten the screen
  (KEY_NOT_PRESSED)
  @color
  M=0
// end if
(IF_END)

// R1: i
// R2: j
// R3: i*32
// for each row 0 ... 255
@R1
M=0
@R3
M=0
(FOR_START_1)
  @R1
  D=M
  @256
  D=D-A
  @FOR_END_1
  D;JGE

  // for each word in row
  @R2
  M=0
  (FOR_START_2)
    @R2
    D=M
    @32
    D=D-A
    @FOR_END_2
    D;JGE

    // write new color into SCREEN memory
    @SCREEN
    D=A
    @R3
    D=D+M
    @R2
    D=D+M
    // R4 is a temp
    @R4
    M=D
    @color
    D=M
    @R4
    A=M
    M=D

    @R2
    M=M+1
    @FOR_START_2
    0;JMP
  (FOR_END_2)

  @R1
  M=M+1
  @32
  D=A
  @R3
  M=M+D
  @FOR_START_1
  0;JMP
(FOR_END_1)

@LOOP_START
0;JMP
