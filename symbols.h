#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// define your own types and function prototypes for the symbol table(s) module below

// define structures for compilation
typedef struct ProgramTable ProgramTable;
typedef struct ClassTable ClassTable;
typedef struct SubroutineTable SubroutineTable;
typedef struct ProgramTableEntry ProgramTableEntry;
typedef struct ClassTableEntry ClassTableEntry;
typedef struct SubroutineTableEntry SubroutineTableEntry;
typedef struct ScopeStack ScopeStack;

// struct for the scope stack
struct ScopeStack
{
    int *bottom;
    int capacity;
    int depth;
};

// struct for the program symbol table
struct ProgramTable
{
    ProgramTableEntry **entries;
    int capacity;
    int count;
};

// struct for the class symbol table
struct ClassTable
{
    ClassTableEntry **entries;
    int capacity;
    int count;
};

// struct for the subroutine symbol table
struct SubroutineTable
{
    SubroutineTableEntry **entries;
    int capacity;
    int count;
};

// struct for entries in the program table
struct ProgramTableEntry
{
    char *name;
    int index;
    ClassTable *table;
};

// struct for entries in the class table
struct ClassTableEntry
{
    char *name;
    char *type;
    char *kind; // "static", "field", "argument", or "var"
    int index;
    SubroutineTable *table;
};

// struct for entries in the subroutine table
struct SubroutineTableEntry
{
    char *name;
    char *type;
    char *kind; // "static", "field", "argument", or "var"
    int index;
};

/*
programSymbolTable
- ClassSymbolTable
-- MethodSymbolTable


*/

/*Function Prototypes*/
void initSymbolTable();
void initScopeStack();
ClassTable *createClassTable();
SubroutineTable *createSubroutineTable();

#endif
