#!/bin/bash

#jack files
jackFiles=(Main.jack Ball.jack Empty.jack EofInComment.jack EofInStr.jack Fraction.jack IllegalSymbol.jack List.jack NewLineInStr.jack OnlyComments.jack Square.jack SquareGame.jack)
#corresponding token files
tokenFiles=(Main.jack_tokens.txt Ball.jack_tokens.txt Empty.jack_tokens.txt EofInComment.jack_tokens.txt EofInStr.jack_tokens.txt Fraction.jack_tokens.txt IllegalSymbol.jack_tokens.txt List.jack_tokens.txt NewLineInStr.jack_tokens.txt OnlyComments.jack_tokens.txt Square.jack_tokens.txt SquareGame.jack_tokens.txt)

clear
printf "Building lexer\n"
gcc lexer.c -o lexer
index=0
for file in ${jackFiles[@]}; do
    printf "Testing with $file\n"
    $(./lexer $file output)
    if [ $? == 0 ]
    then
        diff ${tokenFiles[$index]} output
        if [ $? == 0 ]
        then 
            printf "\tTest Passed!\n"
        fi
    else 
        printf "\tProgram Crashed :(\n"
    fi
    index=$((index + 1))
done
rm lexer output