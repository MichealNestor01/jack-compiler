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

// scope stack keeps track of what table we need to add stuff to
static ScopeStack scope;

// define the program table
static ProgramTable programTable;

// struct for the scope stack
struct ScopeStack
{
    unsigned long *bottom;
    int capacity;
    int depth;
};

// struct for the program symbol table
struct ProgramTable
{
    ProgramTableEntry **entries;
    int capacity;
    int count;
    int parsedOnce;
};

// struct for the class symbol table
struct ClassTable
{
    ClassTableEntry **entries;
    int capacity;
    int count;
    char name[128];
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
    char name[128];
    int index;
    ClassTable *table;
};

// struct for entries in the class table
struct ClassTableEntry
{
    char name[128];
    char type[128];
    char kind[128]; // "static", "field", "argument", or "var"
    int kindIndex;
    int index;
    SubroutineTable *table;
};

// struct for entries in the subroutine table
struct SubroutineTableEntry
{
    char name[128];
    char type[128];
    char kind[128]; // "static", "field", "argument", or "var"
    int kindIndex;
    int index;
};

/*Function Prototypes*/
/*Initialisation*/
void initSymbolTable();
void initScopeStack();
void freeScopeStack();
void incrementProgramTableParsed();
/*Stack Operations*/
void popScope();
void pushScope(unsigned long table);
unsigned long getScopeTop();
unsigned long getScopeClass();
/*Constructors*/
ProgramTableEntry *createProgramTableEntry(char *name, int index);
ClassTableEntry *createClassTableEntry(char *name, char *type, char *kind, int index);
ClassTableEntry *createClassTableEntryWithTable(char *name, char *type, char *kind, int index);
ClassTable *createClassTable();
SubroutineTable *createSubroutineTable();
SubroutineTableEntry *createSubroutineTableEntry(char *name, char *type, char *kind, int index);
/*Getters*/
ProgramTable *getProgramTable();
/*table operators*/
void addToProgramTable(ProgramTableEntry *entry);
void addToClassTable(ClassTable *table, ClassTableEntry *entry);
void addToSubroutineTable(SubroutineTable *table, SubroutineTableEntry *entry);
#endif
