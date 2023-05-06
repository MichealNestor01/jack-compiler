/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include "compiler.h"

int InitCompiler()
{
	// init the symbol table
	initSymbolTable();
	initScopeStack();
	return 1;
}

ParserInfo compile(char *dir_name)
{
	ParserInfo p;
	p.er = none;

	// write your code below

	// only parse the library files the first time
	char *libraryFiles[8] = {"Array.jack", "Keyboard.jack", "Math.jack", "Memory.jack", "Output.jack", "Screen.jack", "String.jack", "Sys.jack"};
	for (int lib = 0; lib < 8; lib++)
	{
		InitParser(libraryFiles[lib]);
		Parse();
		StopParser();
	}

	// printf("PARSED LIBRARIES\n");

	// we have to parse twice
	for (int parseIndex = 0; parseIndex < 2; parseIndex++)
	{
		// open the directory given
		DIR *dirObj = opendir(dir_name);
		if (dirObj == NULL)
		{
			printf("Error: Directory can't be opened\n");
			p.er = lexerErr;
			return p;
		}

		// loop through each file in the directory
		struct dirent *dir;
		while ((dir = readdir(dirObj)) != NULL)
		{
			// skip . and ..
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
				continue;

			// concatonate the filepath and the file name
			char filePath[1024];
			strcpy(filePath, dir_name);
			strcat(filePath, "/");
			strcat(filePath, dir->d_name);

			// print the curent file
			// printf("File Found: \"%s\"\n", dir->d_name);

			// init parser
			InitParser(filePath);
			p = Parse();
			if (p.er != none)
			{
				// printf("Error %d: token: \"%s\" near line %d\n", p.er, p.tk.lx, p.tk.ln);
				closedir(dirObj);
				return p;
			}
			else
			{
				// printf("%s Parsed with no errors\n", dir->d_name);
			}

			// stop the parser
			StopParser();
		}
		closedir(dirObj);
		incrementProgramTableParsed();
	}

	return p;
}

int StopCompiler()
{
	return 1;
}
/*
#ifndef TEST_COMPILER
int main()
{
	InitCompiler();
	ParserInfo p = compile("Pong");
	// PrintError(p);
	StopCompiler();
	return 1;
}
#endif
*/