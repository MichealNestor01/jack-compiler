
/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Symbol Tables Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "symbols.h"

/* initialisation */
void initSymbolTable()
{
    programTable.entries = (ProgramTableEntry **)malloc(10 * sizeof(ProgramTableEntry *));
    programTable.capacity = 9;
    programTable.count = 0;
    programTable.parsedOnce = 0;
}

void incrementProgramTableParsed()
{
    programTable.parsedOnce++;
}

void initScopeStack()
{
    scope.bottom = (unsigned long *)malloc(sizeof(unsigned long) * 30);
    scope.capacity = 29;
    scope.depth = -1;
}

/* scope manipulation functions */
void popScope()
{
    if (scope.depth != 0)
    {
        scope.depth--;
    }
}

void pushScope(unsigned long table)
{
    if (scope.depth == scope.capacity)
    {
        // alocate another 30 spaces
        scope.capacity += 30;
        scope.bottom = (unsigned long *)realloc(scope.bottom, sizeof(unsigned long) * scope.capacity);
    }
    // asign the top of the stack to the given table
    scope.depth++;
    scope.bottom[scope.depth] = table;
}

unsigned long getScopeTop()
{
    return scope.bottom[scope.depth];
}

unsigned long getScopeClass()
{
    return scope.bottom[scope.depth - 1];
}

/* Constructors */
ProgramTableEntry *createProgramTableEntry(char *name, int index)
{
    // allocate memory for entries
    ProgramTableEntry *entry = (ProgramTableEntry *)malloc(sizeof(ProgramTableEntry));
    strcpy(entry->name, name);
    entry->index = index;
    entry->table = createClassTable();
    return entry;
}

ClassTableEntry *createClassTableEntry(char *name, char *type, char *kind, int index)
{
    // printf("CREATING ENTRY\n\tNAME: %s\n\tTYPE: %s\n\tKIND: %s\n\tINDEX: %d\n", name, type, kind, index);
    // allocate memory for entries
    ClassTableEntry *entry = (ClassTableEntry *)malloc(sizeof(ClassTableEntry));
    strcpy(entry->name, name);
    strcpy(entry->type, type);
    strcpy(entry->kind, kind);
    entry->index = index;
    return entry;
}

ClassTableEntry *createClassTableEntryWithTable(char *name, char *type, char *kind, int index)
{
    // printf("CREATING ENTRY\n\tNAME: %s\n\tTYPE: %s\n\tKIND: %s\n\tINDEX: %d\n", name, type, kind, index);
    // allocate memory for entries
    ClassTableEntry *entry = (ClassTableEntry *)malloc(sizeof(ClassTableEntry));
    strcpy(entry->name, name);
    strcpy(entry->type, type);
    strcpy(entry->kind, kind);
    entry->index = index;
    entry->table = createSubroutineTable();
    return entry;
}

SubroutineTableEntry *createSubroutineTableEntry(char *name, char *type, char *kind, int index)
{
    // printf("CREATING ENTRY\n\tNAME: %s\n\tTYPE: %s\n\tKIND: %s\n\tINDEX: %d\n", name, type, kind, index);
    // allocate memory for entries
    SubroutineTableEntry *entry = (SubroutineTableEntry *)malloc(sizeof(SubroutineTableEntry));
    strcpy(entry->name, name);
    strcpy(entry->type, type);
    strcpy(entry->kind, kind);
    entry->index = index;
    return entry;
}

ClassTable *createClassTable()
{
    ClassTable *table = (ClassTable *)malloc(sizeof(ClassTable));
    table->entries = (ClassTableEntry **)malloc(10 * sizeof(ClassTableEntry *));
    table->capacity = 10;
    table->count = 0;
    return table;
}

SubroutineTable *createSubroutineTable()
{
    SubroutineTable *table = (SubroutineTable *)malloc(sizeof(SubroutineTable));
    table->entries = (SubroutineTableEntry **)malloc(10 * sizeof(SubroutineTableEntry *));
    table->capacity = 10;
    table->count = 0;
    return table;
}

/* Getters */
ProgramTable *getProgramTable()
{
    return &programTable;
}

/* Add entry to a table */
void addToProgramTable(ProgramTableEntry *entry)
{
    if (programTable.count == programTable.capacity)
    {
        // alocate another 10 spaces
        programTable.capacity += 10;
        programTable.entries = (ProgramTableEntry **)realloc(programTable.entries, sizeof(ProgramTableEntry *) * programTable.capacity);
    }
    // asign the top of the stack to the given table
    programTable.entries[programTable.count] = entry;
    programTable.count++;
}

void addToClassTable(ClassTable *table, ClassTableEntry *entry)
{
    // work out entry's kind number
    int kindIndex = 0;
    for (int index = 0; index < table->count; index++)
    {
        if (strcmp(table->entries[index]->kind, entry->kind) == 0)
        {
            kindIndex++;
        }
    }
    entry->kindIndex = kindIndex;
    if (table->count == table->capacity)
    {
        table->capacity += 10;
        table->entries = (ClassTableEntry **)realloc(table->entries, sizeof(ClassTableEntry *) * table->capacity);
    }
    table->entries[table->count] = entry;
    table->count++;
}

void addToSubroutineTable(SubroutineTable *table, SubroutineTableEntry *entry)
{
    // work out entry's kind number
    int kindIndex = 0;
    for (int index = 0; index < table->count; index++)
    {
        if (strcmp(table->entries[index]->kind, entry->kind) == 0)
        {
            kindIndex++;
        }
    }
    entry->kindIndex = kindIndex;
    if (table->count == table->capacity)
    {
        table->capacity += 10;
        table->entries = (SubroutineTableEntry **)realloc(table->entries, sizeof(SubroutineTableEntry *) * table->capacity);
    }
    table->entries[table->count] = entry;
    table->count++;
}