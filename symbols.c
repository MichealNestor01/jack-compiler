
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

// define the program table
static ProgramTable programTable = {
    NULL,
    0,
    0,
};

// scope stack keeps track of what table we need to add stuff to
static int *scopeStack;

/* initialisation */

void initSymbolTable()
{
    programTable.entries = (ProgramTableEntry **)malloc(10 * sizeof(ProgramTableEntry *));
    programTable.capacity = 10;
}

void initScopeStack()
{
    scopeStack = (int *)malloc(sizeof(int) * 20);
}

/* creating tables */
ClassTable *createClassTable()
{
    ClassTable *table = (ClassTable *)malloc(sizeof(ClassTable));
    table->entries = (ClassTableEntry **)malloc(10 * sizeof(ClassTableEntry *));
    table->capacity = 10;
    table->entries = 0;
}

SubroutineTable *createSubroutineTable()
{
    SubroutineTable *table = (SubroutineTable *)malloc(sizeof(SubroutineTable));
    table->entries = (SubroutineTableEntry **)malloc(10 * sizeof(SubroutineTableEntry *));
    table->capacity = 10;
    table->entries = 0;
}