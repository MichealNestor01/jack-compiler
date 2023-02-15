/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Micheal Nestor
Student ID: 201492471
Email: sc21mpn@leeds.ac.uk  
Date Work Commenced: 14/02/2023s
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE

typedef struct {
  FILE * filePointer;		// points to the file undergoing analysis
  int initialised; // boolean value
  int currentLine;
} Lexer;

// set default values
static Lexer lexerObj = {NULL, 0, 1};

int isWhiteSpace(unsigned char c) {
  if (c == ' ' || c == '\t' || c == '\r') {
    return 1; // found white space
  }
  return 0; // not white space
}

void skipWhiteSpace() { // skips past all the white space in the file
  unsigned char currentChar;
  while (1) { // go past all white space
    currentChar = fgetc(lexerObj.filePointer);
    if(isWhiteSpace(currentChar) == 0) { // found non white space
      ungetc(currentChar, lexerObj.filePointer); 
      break;
    }
  }
}

int skipComments() {
  unsigned char current = fgetc(lexerObj.filePointer);
  if (current == '/') { // inline comment
    do {
      current = fgetc(lexerObj.filePointer);
      if (current == '\n') lexerObj.currentLine++;
    } while (current != '\n' && current != EOF);
  } else { // multi like comment 
    unsigned char tmp;
    do {
      // search until a '*' is found, then look for a '/'
      tmp = fgetc(lexerObj.filePointer);
      if (tmp == EOF) return 1;
      if (tmp == '\n') lexerObj.currentLine++;
      if (tmp != '*') continue;
      current = fgetc(lexerObj.filePointer);
      if (current == '\n') lexerObj.currentLine++;
    } while (current != '/' && current != EOF);
  }
  if (current == EOF) {
    return 1; // Error
  }
  return 0; // no error
}

void GenerateTokens() {
  if (lexerObj.initialised == 0) return;
  
  while (1) { // loop through the file
    // skip white space
    skipWhiteSpace();

    // check if the there is a line break
    unsigned char current = fgetc(lexerObj.filePointer);
    if (current == '\n') {
      lexerObj.currentLine++;
      continue;
    }

    // check if this is a comment
    if (current == '/') {
      int err = skipComments();
      if (err) return;
      continue;
    }

    
    printf("First non white space: (%c) on line %d\n", current, lexerObj.currentLine);
    break;
  }

}


// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer (char* file_name)
{
  // reset the lexerObj
  lexerObj.filePointer = NULL;
  lexerObj.initialised = 0;

  // Check that the file given is a .jack file
  int lenFileName = strlen(file_name);
  char* fileExtension = ".jack";
  for (
    int extIndex = 0, fnIndex = lenFileName-5; 
    extIndex < 5; 
    extIndex++, fnIndex++
    ) {
    // if the extension does not match return 0, error.
    if (file_name[fnIndex] != fileExtension[extIndex]) return 0; 
  }
  
  // Open the file given, and link it to the static file pointer
  lexerObj.filePointer = fopen(file_name, "r");
  // if the filePointer is null return 0, error.
  if (lexerObj.filePointer == NULL) return 0;

  // All initialisation steps passed
  lexerObj.initialised = 1;
  GenerateTokens();
  return 1;
}


// Get the next token from the source file
Token GetNextToken ()
{
	Token t;
  t.tp = ERR;
  return t;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
  Token t;
  t.tp = ERR;
  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{
	return 0;
}

// do not remove the next line
#ifndef TEST
int main ()
{
	// implement your main function here
  // NOTE: the autograder will not use your main function

  // test the initialiser
  InitLexer("hellowolrd.jack");
  printf("%d\n", lexerObj.initialised);
  InitLexer("LOLOLOL");
  printf("%d\n", lexerObj.initialised);
  InitLexer("Ball.jack");
  printf("%d\n", lexerObj.initialised);
	return 0;
}
// do not remove the next line
#endif
