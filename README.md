# Jack Compiler

## Description

This is a compiler that translates the [Jack language](https://www.csie.ntu.edu.tw/~cyy/courses/introCS/17fall/lectures/handouts/lec13_Jack.pdf) into [Jack VM code](https://www.csie.ntu.edu.tw/~cyy/courses/introCS/17fall/lectures/handouts/lec12_VMII_4up.pdf) using a recursive descent parser. These are the languages that were created for the [nand2tetris](www.nand2tetris.org) course.

This was the primary assessed piece of work for [COMP2932 Compiler Design and Construction](https://webprod3.leeds.ac.uk/catalogue/dynmodules.asp?Y=201819&M=COMP-2932). The code was tested using testscripts provided by Dr Mhd. A. Alsalka as they are not mine they are not included in this repository, but this compiler did recieve 100% of the available marks.

## Compile & Run Instuctions

Compile the program:

```
$ gcc lexer.c parser.c compiler.c symbols.c -o compiler
```

Run the program:

```
./compiler <Jack/Program/Directory>
```

There are few exmaple Jack programs in this repository with some vm code generated by my compiler which you can have a play with.

## Project Reflection

Ultimately
