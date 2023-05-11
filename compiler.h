#ifndef COMPILER_H
#define COMPILER_H

// #define TEST_COMPILER    // uncomment to run the compiler autograder
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include "parser.h"
#include "symbols.h"

static FILE *outputFile;

FILE *getOutputFile();

int InitCompiler();
ParserInfo compile(char *dir_name);
int StopCompiler();

#endif
