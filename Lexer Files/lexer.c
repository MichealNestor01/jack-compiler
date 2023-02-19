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

/*
  TODO LIST:
    - what if eof in string
    - make error tokens work, currently they are seen as identifiers
*/

// true and false macros
#define true 1
#define false 0

// list of all reserved words
const char RESERVED_WORDS[18][12] = {
    "class", "constructor", "method", "function",
    "int", "boolean", "char", "void",
    "let", "do", "if", "else", "while", "return",
    "true", "false", "null", "this"};

typedef struct TokenStreamItem TokenStreamItem;
struct TokenStreamItem
{
  Token *token;
  TokenStreamItem *next;
};

typedef struct
{
  TokenStreamItem *head;
  TokenStreamItem *current;
} TokenStream;

typedef struct
{
  FILE *filePointer; // points to the file undergoing analysis
  char filename[32];
  int initialised; // boolean value
  int currentLine;
  TokenStream stream;
} Lexer;

// set default values
static Lexer lexerObj = {NULL, "", false, 1, {NULL, NULL}};

// convert token type enum to a string
char *getSymbolString(TokenType type)
{
  switch (type)
  {
  case RESWORD:
    return "RESWORD";
  case ID:
    return "ID";
  case INT:
    return "INT";
  case SYMBOL:
    return "SYMBOL";
  case STRING:
    return "STRING";
  case EOFile:
    return "EOFile";
  case ERR:
    return "ERR";
  default:
    return "__Error__";
  }
}
// format a token
void printTokenPointer(Token *token)
{
  printf("< %s, %d, %s, %s >\n",
         token->fl,
         token->ln,
         token->lx,
         getSymbolString(token->tp));
}
void printToken(Token token)
{
  printf("< %s, %d, %s, %s >\n",
         token.fl,
         token.ln,
         token.lx,
         getSymbolString(token.tp));
}

// memory cleanup
void freeTokenStream()
{
  TokenStreamItem *current = lexerObj.stream.head;
  TokenStreamItem *next;
  do
  {
    TokenStreamItem *next = current->next;
    // free(current->token);
    // free(current);
    current = next;
  } while (current != NULL);
}

// checks if a given character is white space
int isWhiteSpace(char c)
{
  if (c == ' ' || c == '\t' || c == '\r')
  {
    return true;
  }
  return false;
}

// checks if a given character is a symbol
int isSymbol(unsigned int c)
{
  if (
      (40 <= c && c <= 47) || // ( ) * + , - . /
      (59 <= c && c <= 62) || // ; < = >
      (c == 91) ||            // [
      (c == 93) ||            // ]
      (123 <= c && c <= 126)  // { | } ~
  )
  {
    return true;
  }
  return false;
}

// this will move the file pointer past white space
void skipWhiteSpace()
{ // skips past all the white space in the file
  char currentChar;
  while (true)
  { // go past all white space
    currentChar = fgetc(lexerObj.filePointer);
    if (!isWhiteSpace(currentChar))
    { // found non white space
      ungetc(currentChar, lexerObj.filePointer);
      break;
    }
  }
}

// this will move the file pointer past comments
int skipComments()
{
  char current = fgetc(lexerObj.filePointer);
  if (current == '/')
  { // inline comment
    do
    {
      current = fgetc(lexerObj.filePointer);
      if (current == '\n')
        lexerObj.currentLine++;
    } while (current != '\n' && current != EOF);
  }
  else if (current == '*')
  { // multi like comment
    char next;
    do
    { // search until a '*' is found, then look for a '/'
      // get the next char
      current = fgetc(lexerObj.filePointer);
      // check if it is a new line or EOF
      if (current == '\n')
        lexerObj.currentLine++;
      else if (current == '*')
      { // possible end of current
        next = fgetc(lexerObj.filePointer);
        if (next != '/')
          ungetc(next, lexerObj.filePointer);
      }
    } while (next != '/' && current != EOF);
  }
  else
  { // not a comment so put the current back
    ungetc(current, lexerObj.filePointer);
    return 2;
  }
  if (current == EOF)
  { // Error
    return 1;
  }
  return 0; // no error
}

// gets the current token pointed at by the file pointer
char *getTokenString(char current, LexErrCodes code)
{
  char *token = (char *)malloc(sizeof(char) * 128);
  if (code != -1)
  { // error, so return related error code
    switch (code)
    {
    case EofInCom:
      return "End of file in comment";
    case NewLnInStr:
      return "New line in string literal";
    case EofInStr:
      return "End of file in string literal";
    case IllSym:
      return "Illegal symbol in source file";
    default:
      break;
    }
  }

  unsigned int index = 0;
  unsigned int tokenIsString = false;
  do
  {
    token[index++] = current;
    if (current == '\"')
    {
      tokenIsString = true;
      break;
    }
    if (isSymbol(current))
      return token;
    current = fgetc(lexerObj.filePointer);
    if (current == EOF)
    {
      ungetc(current, lexerObj.filePointer);
      break;
    }
    else if (isSymbol(current))
    {
      ungetc(current, lexerObj.filePointer);
      break;
    }
  } while (!isWhiteSpace(current));
  if (tokenIsString)
  {
    do
    {
      current = fgetc(lexerObj.filePointer);
      token[index++] = current;
    } while (current != '\"');
    for (int i = 0; i < index - 2; i++)
    { // remove the "'s
      token[i] = token[i + 1];
    }
    // set the end index to point to the first " at the end of the string.
    index -= 2;
  }
  // close the end of the token with EOS
  token[index] = '\0';
  return token;
}

Token *classifyToken(char *tokenString, LexErrCodes code)
{
  unsigned int tokenStringLength = strlen(tokenString);

  // initialise the token
  Token *token = (Token *)malloc(sizeof(Token));
  strncpy(token->lx, tokenString, 128);
  token->lx[tokenStringLength] = '\0';
  token->ln = lexerObj.currentLine;
  strncpy(token->fl, lexerObj.filename, 32);
  token->fl[strlen(lexerObj.filename)] = '\0';

  // check for error
  if (code != -1)
  {
    token->tp = ERR;
    return token;
  }

  // check for string
  if (token->lx[0] == '\"')
  {
    token->tp = STRING;
    return token;
  }

  // check for end of file
  if (strcmp(token->lx, "End of File") == 0)
  {
    token->tp = EOFile;
    return token;
  }

  // check for symbol
  if (tokenStringLength == 1 && isSymbol(token->lx[0]))
  {
    token->tp = SYMBOL;
    return token;
  }

  // check for integer
  if (atoi(token->lx) != 0 || (token->lx[0] == '0' && tokenStringLength == 1))
  {
    token->tp = INT;
    return token;
  }

  // check for reserved word
  for (int index = 0; index < 18; index++)
  {
    if (strcmp(token->lx, RESERVED_WORDS[index]) == 0)
    {
      token->tp = RESWORD;
      return token;
    }
  }

  // else token is an identifier
  token->tp = ID;
  return token;

  // going to need to check for error somehow
}

// generates tokens from the given file
void GenerateTokens()
{
  if (!lexerObj.initialised)
    freeTokenStream();
  int tokens = 0;
  LexErrCodes error = -1;
  char *tokenString;
  while (1)
  { // loop through the file
    // skip white space
    skipWhiteSpace();
    char current = fgetc(lexerObj.filePointer);
    error = -1;

    if (current == '\n')
    { // check if the there is a line break
      lexerObj.currentLine++;
      continue;
    }

    if (current == '/')
    { // check if this is a comment
      int err = skipComments();
      if (err == 0)
      {
        continue; // comment skipped
      }
      else if (err == 1)
      {
        error = EofInCom;
      }
    }

    // we have reached something to tokenise
    int end = false;
    if (current == EOF)
    {
      tokenString = "End of File";
      end = true;
    }
    else
    {
      tokenString = getTokenString(current, error);
    }

    // classify token
    Token *token = classifyToken(tokenString, error);

    // add token to the linked token list
    TokenStreamItem *item = (TokenStreamItem *)malloc(sizeof(TokenStreamItem));
    item->token = token;
    if (lexerObj.stream.head == NULL)
    {
      lexerObj.stream.current = item;
      lexerObj.stream.current->next = NULL;
      lexerObj.stream.head = item;
    }
    else
    {
      lexerObj.stream.current->next = item;
      lexerObj.stream.current = item;
      lexerObj.stream.current->next = NULL;
    }

    // printf("Token %d: (%s) on line %d\n", tokens, tokenString, lexerObj.currentLine);
    if (end)
      break;
    tokens++;
  }
  lexerObj.stream.current = lexerObj.stream.head;
}

// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer(char *file_name)
{
  if (lexerObj.initialised)
  { // free the old token stream
    freeTokenStream();
  }

  int lenFileName = strlen(file_name);
  // reset the lexerObj
  lexerObj.filePointer = NULL;
  strncpy(lexerObj.filename, file_name, 32);
  lexerObj.initialised = 0;

  // Check that the file given is a .jack file
  char *fileExtension = ".jack";
  for (
      int extIndex = 0, fnIndex = lenFileName - 5;
      extIndex < 5;
      extIndex++, fnIndex++)
  { // if the extension does not match return 0, error.
    if (file_name[fnIndex] != fileExtension[extIndex])
      return 0;
  }

  // Open the file given, and link it to the static file pointer
  lexerObj.filePointer = fopen(file_name, "r");
  // if the filePointer is null return 0, error.
  if (lexerObj.filePointer == NULL)
    return 0;

  // All initialisation steps passed
  lexerObj.initialised = true;
  GenerateTokens();
  fclose(lexerObj.filePointer);
  return 1;
}

// Get the next token from the source file
Token GetNextToken()
{ // This doesn't deal with the end of the file
  Token t = *(lexerObj.stream.current->token);
  if (lexerObj.stream.current->next != NULL)
  {
    lexerObj.stream.current = lexerObj.stream.current->next;
  }
  return t;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken()
{
  return *(lexerObj.stream.current->token);
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer()
{
  return 0;
}

// do not remove the next line
#ifndef TEST
int main(int argc, char **argv)
{
  // implement your main function here
  // NOTE: the autograder will not use your main function

  // test the initialiser

  InitLexer(argv[1]);

  FILE *output = fopen(argv[2], "w");
  do
  {
    Token token = GetNextToken();
    fprintf(output, "< %s, %d, %s, %s >\n",
            token.fl,
            token.ln,
            token.lx,
            getSymbolString(token.tp));
    if (token.tp == EOFile)
    {
      break;
    }
  } while (true);
  fclose(output);

  return 0;

  freeTokenStream();

  printf("\n\n\n\nNEW INIT\n\n\n\n");

  InitLexer("EofInComment.jack");

  while (PeekNextToken().tp != EOFile)
  {
    printToken(GetNextToken());
  }

  freeTokenStream();

  return 0;
}
// do not remove the next line
#endif
