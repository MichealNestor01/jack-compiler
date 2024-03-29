#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "compiler.h"

// error function
void error(char *s)
{
	printf("Error: %s", s);
	exit(1);
}

// output buffer
char outputBuffer[128];

// if and while counters
int ifCount;
int whileCount;

// no error parser info
ParserInfo InfoNoError;

// code gen funtions
typedef struct
{
	char type[128];
	char kind[128];
	int kindIndex;
	int isLocal;
} TokenContext;

TokenContext *getTokenContext(Token *token)
{
	TokenContext *context = (TokenContext *)malloc(sizeof(TokenContext));
	ClassTable *classTable = (ClassTable *)getScopeClass();
	SubroutineTable *subTable = (SubroutineTable *)getScopeTop();
	for (int i = 0; i < subTable->count; i++)
	{
		if (strcmp(subTable->entries[i]->name, token->lx) == 0)
		{
			strcpy(context->type, subTable->entries[i]->type);
			strcpy(context->kind, subTable->entries[i]->kind);
			context->kindIndex = subTable->entries[i]->kindIndex;
			context->isLocal = 1;
			return context;
		}
	}
	context->isLocal = 0;
	for (int i = 0; i < classTable->count; i++)
	{
		if (strcmp(classTable->entries[i]->name, token->lx) == 0)
		{
			strcpy(context->type, classTable->entries[i]->type);
			strcpy(context->kind, classTable->entries[i]->kind);
			context->kindIndex = classTable->entries[i]->kindIndex;
			return context;
		}
	}
	return context;
}
// symbol table functions
ParserInfo addTokenToProgramTable(Token *token)
{
	// Check this class has not already been created
	int index = 0;
	ProgramTable *programTable = getProgramTable();
	for (; index < programTable->count; index++)
	{
		if (strcmp(programTable->entries[index]->name, token->lx) == 0)
		{
			// class aready defined
			return (ParserInfo){redecIdentifier, *token};
		}
	}
	// create an entry for this class in the program table
	ProgramTableEntry *entry = createProgramTableEntry(token->lx, index);
	addToProgramTable(entry);
	// push the current classTable to the top of the scope
	pushScope((unsigned long)entry->table);
	return InfoNoError;
}

void pushClassToScope(Token *token)
{
	ProgramTable *programTable = getProgramTable();
	for (int index = 0; index < programTable->count; index++)
	{
		if (strcmp(programTable->entries[index]->name, token->lx) == 0)
		{
			pushScope((unsigned long)programTable->entries[index]->table);
			return;
		}
	}
}

ParserInfo addVarTokenToClassTable(Token *token, char *typeString, char *kindString)
{
	// validate that this var has not already been defined
	ClassTable *classTable = (ClassTable *)getScopeTop();
	int index = 0;
	for (; index < classTable->count; index++)
	{
		if (strcmp(classTable->entries[index]->name, token->lx) == 0)
		{
			// class aready defined
			return (ParserInfo){redecIdentifier, *token};
		}
	}
	// create an entry for this var in the class table
	ClassTableEntry *entry = createClassTableEntry(token->lx, typeString, kindString, index);
	addToClassTable(classTable, entry);
	return InfoNoError;
}

ParserInfo addSubTokenToClassTable(Token *token, char *typeString, char *kindString)
{
	ClassTable *classTable = (ClassTable *)getScopeTop();
	int index = 0;
	for (; index < classTable->count; index++)
	{
		if (strcmp(classTable->entries[index]->name, token->lx) == 0)
		{
			// class aready defined
			return (ParserInfo){redecIdentifier, *token};
		}
	}
	// create an entry for this var in the class table
	ClassTableEntry *entry = createClassTableEntryWithTable(token->lx, typeString, kindString, index);
	addToClassTable(classTable, entry);
	pushScope((unsigned long)entry->table);
	return InfoNoError;
}
void pushSubToScope(Token *token)
{
	ClassTable *classTable = (ClassTable *)getScopeTop();
	for (int index = 0; index < classTable->count; index++)
	{
		if (strcmp(classTable->entries[index]->name, token->lx) == 0)
		{
			pushScope((unsigned long)classTable->entries[index]->table);
			return;
		}
	}
}
ParserInfo addTokenToSubroutineTable(Token *token, char *typeString, char *kindString)
{
	// check that this arg has not already been defined
	SubroutineTable *table = (SubroutineTable *)getScopeTop();
	int index = 0;
	for (; index < table->count; index++)
	{
		if (strcmp(table->entries[index]->name, token->lx) == 0)
		{
			// argument already defined
			return (ParserInfo){redecIdentifier, *token};
		}
	}
	// create an entry for this argument in the table
	SubroutineTableEntry *entry = createSubroutineTableEntry(token->lx, typeString, kindString, index);
	addToSubroutineTable(table, entry);
	return InfoNoError;
}
ParserInfo isVarInScope(Token *token)
{
	//  get the current subroutine table
	SubroutineTable *subTable = (SubroutineTable *)getScopeTop();
	//  get the current class table
	ClassTable *classTable = (ClassTable *)getScopeClass();
	// check that this identifier exists, first check sub table,
	// then check the class table
	for (int index = 0; index < subTable->count; index++)
	{
		if (strcmp(subTable->entries[index]->name, token->lx) == 0)
			return InfoNoError;
	}
	for (int index = 0; index < classTable->count; index++)
	{
		if (strcmp(classTable->entries[index]->name, token->lx) == 0)
			return InfoNoError;
	}
	// identifier has not been found
	return (ParserInfo){undecIdentifier, *token};
}

ParserInfo isSubInScope(Token *token)
{
	// get the current class table
	ClassTable *classTable = (ClassTable *)getScopeClass();
	for (int index = 0; index < classTable->count; index++)
	{
		// printf("Checking %s against: %s of kind %s\n", token->lx, classTable->entries[index]->name, classTable->entries[index]->kind);
		if (strcmp(classTable->entries[index]->name, token->lx) == 0)
			return InfoNoError;
	}
	// identifier has not been found
	return (ParserInfo){undecIdentifier, *token};
}

ClassTable *getMatchingClass(Token *token)
{
	// printf("Trying to find table: %s\n", token->lx);
	ProgramTable *programTable = getProgramTable();
	for (int index = 0; index < programTable->count; index++)
	{
		// printf("\tFound class table: %s\n", programTable->entries[index]->name);
		if (strcmp(programTable->entries[index]->name, token->lx) == 0)
			return programTable->entries[index]->table;
	}
	// printf("Not found table: %s\n", token->lx);
	//  table not found
	return (ClassTable *)0;
}

ParserInfo isSubInClass(Token *token, ClassTable *table)
{
	// printf("Does %s exist in the found table\n", token->lx);
	for (int index = 0; index < table->count; index++)
	{
		// printf("Checking %s against: %s of kind %s\n", token->lx, table->entries[index]->name, table->entries[index]->kind);
		if (strcmp(table->entries[index]->name, token->lx) == 0)
			return InfoNoError;
	}
	// identifier has not been found
	return (ParserInfo){undecIdentifier, *token};
}
ParserInfo isCallValid(Token *object, Token *subroutine)
{
	// get the type of the object
	char *type = "";
	//  get the current subroutine tab/le
	SubroutineTable *subTable = (SubroutineTable *)getScopeTop();
	//  get the current class table
	ClassTable *classTable = (ClassTable *)getScopeClass();
	for (int index = 0; index < subTable->count; index++)
	{
		if (strcmp(subTable->entries[index]->name, object->lx) == 0)
		{
			type = subTable->entries[index]->type;
			break;
		}
	}
	if (strcmp(type, "") == 0)
	{
		for (int index = 0; index < classTable->count; index++)
		{
			if (strcmp(classTable->entries[index]->name, object->lx) == 0)
			{
				type = classTable->entries[index]->type;
				break;
			}
		}
	}
	// find the matching class table for the type of object
	ProgramTable *programTable = getProgramTable();
	for (int index = 0; index < programTable->count; index++)
	{
		if (strcmp(programTable->entries[index]->name, type) == 0)
		{
			classTable = programTable->entries[index]->table;
			break;
		}
	}
	// now look through the class table for the subroutine
	for (int index = 0; index < classTable->count; index++)
	{
		if (strcmp(classTable->entries[index]->name, subroutine->lx) == 0)
		{
			return InfoNoError;
		}
	}
	return (ParserInfo){undecIdentifier, *subroutine};
}

// class defnitions
ParserInfo class();
ParserInfo memberDeclar();
ParserInfo classVarDeclar();
ParserInfo type();
ParserInfo subroutineDeclar();
ParserInfo paramList();
ParserInfo subroutineBody();
ParserInfo wrappedZeroOrMoreStatements();
ParserInfo statement();
ParserInfo varDeclarStatement();
ParserInfo letStatement();
ParserInfo ifStatement();
ParserInfo whileStatement();
ParserInfo wrappedExpression();
ParserInfo doStatement();
ParserInfo subroutineCall();
ParserInfo expressionList();
ParserInfo returnStatement();
ParserInfo expression();
ParserInfo relationalExpression();
ParserInfo arithmeticExpression();
ParserInfo term();
ParserInfo factor();
ParserInfo dotIdentifier();
ParserInfo wrappedExpressionList();
ParserInfo operand();

// Class Grammar:
// class→class identifier { { memeberDeclar } }
ParserInfo class()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	ParserInfo info;
	// class
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "class") != 0)
		return (ParserInfo){classExpected, next_token};
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	// only add identifiers in the first parse
	if (parsedOnce == 0)
	{
		info = addTokenToProgramTable(&next_token);
		if (info.er != none)
			return info;
	}
	else
	{
		pushClassToScope(&next_token);
	}
	// {
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "{") != 0)
		return (ParserInfo){openBraceExpected, next_token};
	// {memberdeDeclar}
	next_token = PeekNextToken();
	while ((strcmp(next_token.lx, "static") *
			strcmp(next_token.lx, "field") *
			strcmp(next_token.lx, "constructor") *
			strcmp(next_token.lx, "function") *
			strcmp(next_token.lx, "method")) == 0)
	{
		info = memberDeclar();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
	}
	// }
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "}") != 0)
		return (ParserInfo){closeBraceExpected, next_token};
	// remove the class from the scope
	popScope();
	return InfoNoError;
}
// memberDeclar→classVarDeclar | subroutineDeclar
ParserInfo memberDeclar()
{
	Token next_token = PeekNextToken();
	// try classVarDeclare
	// static|field
	if ((strcmp(next_token.lx, "static") *
		 strcmp(next_token.lx, "field")) == 0)
	{
		return classVarDeclar();
	}
	// else try subroutineDeclare
	// constructor|function|method
	if ((strcmp(next_token.lx, "constructor") *
		 strcmp(next_token.lx, "function") *
		 strcmp(next_token.lx, "method")) == 0)
	{
		return subroutineDeclar();
	}
	return (ParserInfo){syntaxError, next_token};
}
// classVarDeclar→(static|field) type identifier {, identifier};
ParserInfo classVarDeclar()
{
	char *kindString = PeekNextToken().lx;
	int parsedOnce = getProgramTable()->parsedOnce;
	// static|field
	Token next_token = GetNextToken();
	if ((strcmp(next_token.lx, "static") *
		 strcmp(next_token.lx, "field")) != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	char *typeString = PeekNextToken().lx;
	// type
	ParserInfo info = type();
	if (info.er != none)
		return info;
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
		return (ParserInfo){syntaxError, next_token};
	// only add identifiers in the first parse
	if (parsedOnce == 0)
	{
		info = addVarTokenToClassTable(&next_token, typeString, kindString);
		if (info.er != none)
			return info;
	}
	// {, identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ,
		GetNextToken();
		// identifier
		Token next_token = GetNextToken();
		if (next_token.tp != ID)
			return (ParserInfo){idExpected, next_token};
		// only add identifiers in the first parse
		if (parsedOnce == 0)
		{
			info = addVarTokenToClassTable(&next_token, typeString, kindString);
			if (info.er != none)
				return info;
		}
	}
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
		return (ParserInfo){semicolonExpected, next_token};
	return InfoNoError;
}
// type→int|char|boolean|identifier
ParserInfo type()
{
	ProgramTable *programTable = getProgramTable();
	// int|char|boolean|identifier
	Token next_token = GetNextToken();
	if (next_token.tp == ID && programTable->parsedOnce)
	{
		// check that the identifier is a valid type
		for (int index = 0; index < programTable->count; index++)
		{
			if (strcmp(programTable->entries[index]->name, next_token.lx) == 0)
			{
				return InfoNoError;
			}
		}
		return (ParserInfo){undecIdentifier, next_token};
	}
	if (next_token.tp == ID ||
		(strcmp(next_token.lx, "int") *
		 strcmp(next_token.lx, "char") *
		 strcmp(next_token.lx, "boolean")) == 0)
	{
		return InfoNoError;
	}
	// should this be idExpected or Syntax error?
	return (ParserInfo){idExpected, next_token};
}
// subroutineDeclar→( constructor|funtoin|method)( type | void ) identifier( paramList ) subroutineBody
ParserInfo subroutineDeclar()
{
	ifCount = 0;
	whileCount = 0;
	ParserInfo info;
	char *kindString = PeekNextToken().lx;
	int parsedOnce = getProgramTable()->parsedOnce;
	// (constructor|function|method)
	Token next_token = GetNextToken();
	if ((strcmp(next_token.lx, "constructor") *
		 strcmp(next_token.lx, "function") *
		 strcmp(next_token.lx, "method")) != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	char *typeString = PeekNextToken().lx;
	//  ( type | void )
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "void") == 0)
	{
		// eat void
		GetNextToken();
	}
	else
	{
		// type
		info = type();
		if (info.er != none)
			return info;
	}
	//  identifier
	next_token = GetNextToken();
	Token first_token = next_token;
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	// only add identifiers in the first parse
	if (parsedOnce == 0)
	{
		info = addSubTokenToClassTable(&next_token, typeString, kindString);
		if (info.er != none)
			return info;
	}
	//  (
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
		return (ParserInfo){openParenExpected, next_token};
	//  check for closed brackets before checking param list
	next_token = PeekNextToken();
	int argCount = 0;
	if (strcmp(next_token.lx, ")") != 0)
	{
		// paramList
		argCount++;
		parsedOnce = getProgramTable()->parsedOnce;
		next_token = PeekNextToken();
		if (next_token.tp == ID ||
			(strcmp(next_token.lx, "int") *
			 strcmp(next_token.lx, "char") *
			 strcmp(next_token.lx, "boolean")) == 0)
		{
			char *typeString = PeekNextToken().lx;
			// type
			ParserInfo info = type();
			if (info.er != none)
			{
				return info;
			}
			//  indentifier
			next_token = GetNextToken();
			if (next_token.tp != ID)
			{
				return (ParserInfo){idExpected, next_token};
			}
			// only add identifiers in the first parse
			if (parsedOnce == 0)
			{
				info = addTokenToSubroutineTable(&next_token, typeString, "argument");
				if (info.er != none)
					return info;
			}
			//  {, type identifier }
			while (strcmp(PeekNextToken().lx, ",") == 0)
			{
				argCount++;
				//  eat the ,
				GetNextToken();
				char *typeString = PeekNextToken().lx;
				// type
				ParserInfo info = type();
				if (info.er != none)
				{
					return info;
				}
				//  identifier
				Token next_token = GetNextToken();
				if (next_token.tp != ID)
				{
					return (ParserInfo){idExpected, next_token};
				}
				// only add identifiers in the first parse
				if (parsedOnce == 0)
				{
					info = addTokenToSubroutineTable(&next_token, typeString, "argument");
					if (info.er != none)
						return info;
				}
			}
		}
	}
	// )
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
	}
	if (parsedOnce)
	{
		char *className = ((ClassTable *)getScopeTop())->name;
		pushSubToScope(&first_token);
		// write "function class.subName varCount" to the output file
		int varCount = 0;
		SubroutineTable *table = (SubroutineTable *)getScopeTop();
		for (int i = 0; i < table->count; i++)
		{
			if (strcmp(table->entries[i]->kind, "var") == 0)
				varCount++;
		}
		if (strcmp(kindString, "constructor") == 0)
		{ // class fields are arguments to the constructor
			argCount = 0;
			ClassTable *classTable = (ClassTable *)getScopeClass();
			for (int i = 0; i < classTable->count; i++)
			{
				if (strcmp(classTable->entries[i]->kind, "field") == 0)
					argCount++;
			}
		}
		FILE *outputFile = getOutputFile();
		fprintf(outputFile, "function %s.%s %d\n", className, first_token.lx, varCount);
		if (strcmp(kindString, "constructor") == 0)
			fprintf(outputFile, "push constant %d\ncall Memory.alloc 1\npop pointer 0\n", argCount);
		else if (strcmp(kindString, "method") == 0)
			fprintf(outputFile, "push argument 0\npop pointer 0\n");
	}
	// subroutineBody
	info = subroutineBody();
	if (info.er != none)
	{
		return info;
	}
	popScope();
	return InfoNoError;
}
// paramList→(type identifier {, type identifier })|ϵ
ParserInfo paramList()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	Token next_token = PeekNextToken();
	if (next_token.tp == ID ||
		(strcmp(next_token.lx, "int") *
		 strcmp(next_token.lx, "char") *
		 strcmp(next_token.lx, "boolean")) == 0)
	{
		char *typeString = PeekNextToken().lx;
		// type
		ParserInfo info = type();
		if (info.er != none)
		{
			return info;
		}
		//  indentifier
		next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
		}
		// only add identifiers in the first parse
		if (parsedOnce == 0)
		{
			info = addTokenToSubroutineTable(&next_token, typeString, "argument");
			if (info.er != none)
				return info;
		}
		//  {, type identifier }
		while (strcmp(PeekNextToken().lx, ",") == 0)
		{
			//  eat the ,
			GetNextToken();
			char *typeString = PeekNextToken().lx;
			// type
			ParserInfo info = type();
			if (info.er != none)
			{
				return info;
			}
			//  identifier
			Token next_token = GetNextToken();
			if (next_token.tp != ID)
			{
				return (ParserInfo){idExpected, next_token};
			}
			// only add identifiers in the first parse
			if (parsedOnce == 0)
			{
				info = addTokenToSubroutineTable(&next_token, typeString, "argument");
				if (info.er != none)
					return info;
			}
		}
	}
	return InfoNoError;
}
// subroutineBody→ { { statement } }
ParserInfo subroutineBody()
{
	return wrappedZeroOrMoreStatements();
}
// Statement Grammar:
// wrappedZeroOrMoreStatements → { { statement } }
ParserInfo wrappedZeroOrMoreStatements()
{
	// {
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "{") != 0)
	{
		return (ParserInfo){openBraceExpected, next_token};
	}
	// {statement} 0 or more statements
	ParserInfo info;
	next_token = PeekNextToken();
	while (
		(strcmp(next_token.lx, "var") *
		 strcmp(next_token.lx, "let") *
		 strcmp(next_token.lx, "if") *
		 strcmp(next_token.lx, "while") *
		 strcmp(next_token.lx, "do") *
		 strcmp(next_token.lx, "return")) ==
		0)
	{
		info = statement();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
	}
	// }
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "}") != 0)
	{
		return (ParserInfo){closeBraceExpected, next_token};
	}
	return InfoNoError;
}
// statement → varDeclarStatement | letStatemnt | ifStatement | whileStatement | doStatement | returnStatemnt
ParserInfo statement()
{

	Token next_token = PeekNextToken();
	if (strcmp(next_token.lx, "var") == 0)
	{
		return varDeclarStatement();
	}
	else if (strcmp(next_token.lx, "let") == 0)
	{
		return letStatement();
	}
	else if (strcmp(next_token.lx, "if") == 0)
	{
		return ifStatement();
	}
	else if (strcmp(next_token.lx, "while") == 0)
	{
		return whileStatement();
	}
	else if (strcmp(next_token.lx, "do") == 0)
	{
		return doStatement();
	}
	else if (strcmp(next_token.lx, "return") == 0)
	{
		return returnStatement();
	}
	return (ParserInfo){syntaxError, next_token};
}
// varDeclarStatement→var type identifier {, identifier };
ParserInfo varDeclarStatement()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	// var
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "var") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	char *typeString = PeekNextToken().lx;
	// type
	ParserInfo info = type();
	if (info.er != none)
	{
		return info;
	}
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	// check that the identifier has not already been defined
	if (parsedOnce == 0)
	{
		info = addTokenToSubroutineTable(&next_token, typeString, "var");
		if (info.er != none)
			return info;
	}
	// {, identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ","
		GetNextToken();
		// identifier
		next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
		}
		if (parsedOnce == 0)
		{
			// check that the identifier has not already been defined
			info = addTokenToSubroutineTable(&next_token, typeString, "var");
			if (info.er != none)
				return info;
		}
	}
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}

	return InfoNoError;
}
// letStatement → let identifier [ [ expression ] ] = expression ;
ParserInfo letStatement()
{
	ParserInfo info;
	int parsedOnce = getProgramTable()->parsedOnce;
	// let
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "let") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// identifier
	next_token = GetNextToken();
	TokenContext *firstTokenContext = getTokenContext(&next_token);
	FILE *outputFile = getOutputFile();
	char letTarget[128];
	strcpy(letTarget, next_token.lx);
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	// check that this identifier exists
	if (parsedOnce == 0)
	{
		info = isVarInScope(&next_token);
		if (info.er != 0)
			return info;
	}
	// [ [identifier] ]
	int isIndexed = 0;
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "[") == 0)
	{
		isIndexed = 1;
		// eat the "["
		GetNextToken();
		// expression
		info = expression();
		if (info.er != none)
		{
			return info;
		}
		// "]"
		next_token = GetNextToken();
		if (strcmp(next_token.lx, "]") != 0)
		{
			return (ParserInfo){syntaxError, next_token};
		}
		if (parsedOnce)
		{
			if (firstTokenContext->isLocal)
			{
				fprintf(outputFile, "push local %d\nadd\n", firstTokenContext->kindIndex);
			}
			else
			{
				if (strcmp(firstTokenContext->kind, "static") == 0)
					fprintf(outputFile, "push static %d\n", firstTokenContext->kindIndex);
				else
					fprintf(outputFile, "push this %d\nadd\n", firstTokenContext->kindIndex);
			}
		}
	}
	// =
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "=") != 0)
	{
		return (ParserInfo){equalExpected, next_token};
	}
	// expression
	info = expression();
	if (info.er != none)
	{
		return info;
	}
	// write the code to assign the current register value
	// to the target
	if (parsedOnce)
	{
		FILE *outputFile = getOutputFile();
		int found = 0;
		if (firstTokenContext->isLocal)
		{
			if (strcmp(firstTokenContext->kind, "argument") == 0)
				fprintf(outputFile, "pop argument %d\n", firstTokenContext->kindIndex);
			else
			{
				if (strcmp(firstTokenContext->type, "Array") == 0 && isIndexed)
				{
					fprintf(outputFile, "pop temp 0\npop pointer 1\npush temp 0\npop that 0\n");
				}
				else
				{
					fprintf(outputFile, "pop local %d\n", firstTokenContext->kindIndex);
				}
			}
		}
		else
		{
			if (strcmp(firstTokenContext->kind, "static") == 0)
				fprintf(outputFile, "pop static %d\n", firstTokenContext->kindIndex);
			else
				fprintf(outputFile, "pop this %d\n", firstTokenContext->kindIndex);
		}
	}
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}
	free(firstTokenContext);
	return InfoNoError;
}
// ifStatement→if ( expression ) { { statement } } [ else { { statement } } ]
ParserInfo ifStatement()
{
	int localIfCount = ifCount;
	ifCount++;
	// if
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "if") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// ( expression )
	ParserInfo info = wrappedExpression();
	if (info.er != none)
		return info;
	// write the if statement code
	FILE *outputFile = getOutputFile();
	int parsedOnce = getProgramTable()->parsedOnce;
	if (parsedOnce)
		fprintf(outputFile, "if-goto IF_TRUE%d\ngoto IF_FALSE%d\nlabel IF_TRUE%d\n", localIfCount, localIfCount, localIfCount);
	//   { { statement } }
	info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
	}

	//   [ else { { statement } } ]
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "else") == 0)
	{
		if (parsedOnce)
			fprintf(outputFile, "goto IF_END%d\nlabel IF_FALSE%d\n", localIfCount, localIfCount);
		// eat else
		GetNextToken();
		// { { statement } }
		info = wrappedZeroOrMoreStatements();
		if (info.er != none)
		{
			return info;
		}
		if (parsedOnce)
			fprintf(outputFile, "label IF_END%d\n", localIfCount);
	}
	else
	{
		if (parsedOnce)
			fprintf(outputFile, "label IF_FALSE%d\n", localIfCount);
	}
	return InfoNoError;
}
// whileStatement → while ( expression ) { { statement } }
ParserInfo whileStatement()
{
	int localWhileCount = whileCount;
	whileCount++;
	// while
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "while") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	FILE *outputFile = getOutputFile();
	int parsedOnce = getProgramTable()->parsedOnce;
	if (parsedOnce)
	{
		fprintf(outputFile, "label WHILE_EXP%d\n", localWhileCount);
	}
	// ( expression )
	ParserInfo info = wrappedExpression();
	if (info.er != none)
		return info;
	if (parsedOnce)
	{
		fprintf(outputFile, "not\nif-goto WHILE_END%d\n", localWhileCount);
	}
	// { {statement} }
	info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
	}
	if (parsedOnce)
	{
		fprintf(outputFile, "goto WHILE_EXP%d\nlabel WHILE_END%d\n", localWhileCount, localWhileCount);
	}
	// successfully parsed
	return InfoNoError;
}
// doStatement → do subroutineCall ;
ParserInfo doStatement()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	FILE *outputFile = getOutputFile();
	// do
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "do") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	strcpy(outputBuffer, "call ");
	// subroutineCall
	ParserInfo info = subroutineCall();
	if (info.er != none)
		return info;
	if (parsedOnce)
		fprintf(outputFile, "%s\npop temp 0\n", outputBuffer);
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}

	return InfoNoError;
};
// subroutineCall → identifier [.identifier] ( expressionList )
ParserInfo subroutineCall()
{
	ParserInfo info;
	int parsedOnce = getProgramTable()->parsedOnce;
	int argCount = 0;
	// identifier
	Token first_token = GetNextToken();
	if (first_token.tp != ID)
	{
		return (ParserInfo){idExpected, first_token};
	}

	Token next_token = PeekNextToken();
	// [ .identifier ]
	if (strcmp(next_token.lx, ".") == 0)
	{
		int isClass = (unsigned long)getMatchingClass(&first_token);
		if (isClass)
		{
			strcat(outputBuffer, first_token.lx);
		}
		else
		{
			// push object var
			if (parsedOnce)
			{

				FILE *outputFile = getOutputFile();
				TokenContext *firstTokenContext = getTokenContext(&first_token);
				if (firstTokenContext->isLocal)
				{
					if (strcmp(firstTokenContext->kind, "var") == 0)
					{
						argCount++;
						fprintf(outputFile, "push local %d\n", firstTokenContext->kindIndex);
					}
					else
					{
						fprintf(outputFile, "push %s %d\n", firstTokenContext->kind, firstTokenContext->kindIndex);
					}
				}
				else
				{
					argCount++;
					if (strcmp(firstTokenContext->kind, "static") == 0)
						fprintf(outputFile, "push static %d\n", firstTokenContext->kindIndex);
					else
						fprintf(outputFile, "push this %d\n", firstTokenContext->kindIndex);
				}
				// then cat class name
				strcat(outputBuffer, firstTokenContext->type);
				free(firstTokenContext);
			}
		}

		strcat(outputBuffer, ".");
		// info = dotIdentifier();
		// if (info.er != none)
		//	return info;
		next_token = GetNextToken();
		// .
		if (strcmp(next_token.lx, ".") != 0)
			return (ParserInfo){syntaxError, next_token};
		// identifier
		next_token = GetNextToken();
		if (next_token.tp != ID)
			return (ParserInfo){idExpected, next_token};
		strcat(outputBuffer, next_token.lx);
		// if (strcmp(next_token.lx, "print") == 0)
		//	argCount++;
		//  first check if the first idenfifier exists in scope
		//
		//  check if the first identifier is a class that has been parsed
		//  and then check if the current identifier exists in that scope
		if (parsedOnce)
		{
			info = isVarInScope(&first_token);
			// printf("Checked is var in scope\n");
			if (info.er != none)
			{
				ClassTable *table = getMatchingClass(&first_token);
				if (table == 0)
					return (ParserInfo){undecIdentifier, first_token};
				info = isSubInClass(&next_token, table);
				if (info.er != none)
					return info;
			}
			else
			{
				info = isCallValid(&first_token, &next_token);
				if (info.er != none)
					return info;
			}
		}
	}
	else
	{
		argCount++;
		// check if subroutine is in scope
		if (parsedOnce)
		{
			info = isSubInScope(&first_token);
			if (info.er != none)
				return info;
			// get the current class name to prepend the token
			FILE *outputFile = getOutputFile();
			fprintf(outputFile, "push pointer 0\n");
			ClassTable *table = (ClassTable *)getScopeClass();
			strcat(outputBuffer, table->name);
			strcat(outputBuffer, ".");
			strcat(outputBuffer, first_token.lx);
		}
	}

	// ( expressionList )
	// (
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
	{
		return (ParserInfo){openParenExpected, next_token};
	}
	// check for ) skip extra recursion
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, ")") == 0)
	{
		GetNextToken();
	}
	else
	{
		argCount++;
		// expressionList
		info = expression();
		if (info.er != none)
			return info;
		// {, expression }
		while (strcmp(PeekNextToken().lx, ",") == 0)
		{
			argCount++;
			// eat the ","
			GetNextToken();
			// expression
			info = expression();
			if (info.er != none)
				return info;
		}
		// )
		next_token = GetNextToken();
		if (strcmp(next_token.lx, ")") != 0)
		{
			return (ParserInfo){closeParenExpected, next_token};
		}
	}
	char count[3] = " 0\0";
	count[1] += argCount;
	strcat(outputBuffer, count);
	// printf("Calling %s\n", outputBuffer);
	return InfoNoError;
}
// expressoinList → expression {, expression }|ϵ
ParserInfo expressionList()
{
	ParserInfo info = expression();
	if (info.er != none)
		return info;
	// {, expression }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ","
		GetNextToken();
		// expression
		ParserInfo info = expression();
		if (info.er != none)
			return info;
	}

	return InfoNoError;
};
// returnStatement → return [ expression ];
ParserInfo returnStatement()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	FILE *outputFile = getOutputFile();
	Token next_token = GetNextToken();
	// return
	if (strcmp(next_token.lx, "return") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	next_token = PeekNextToken();
	// check for ; first to avoid unecessary checks
	if (strcmp(next_token.lx, ";") == 0)
	{
		GetNextToken();
		if (parsedOnce)
			fprintf(outputFile, "push constant 0\nreturn\n");
		return InfoNoError;
	}

	// [ expression ]
	if ((strcmp(next_token.lx, "-") *
		 strcmp(next_token.lx, "~") *
		 strcmp(next_token.lx, "(")) == 0 ||
		next_token.tp == INT ||
		next_token.tp == STRING ||
		next_token.tp == RESWORD ||
		next_token.tp == ID)
	{
		ParserInfo info = expression();
		if (info.er != none)
			return info;
	}
	if (parsedOnce)
		fprintf(outputFile, "return\n");
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}

	return InfoNoError;
}
// Expressions Grammar:
// expresion→ relationalExpression {( & | | ) relationalExpression }
ParserInfo expression()
{
	// relationalExpression
	FILE *outputFile = getOutputFile();
	int parsedOnce = getProgramTable()->parsedOnce;
	ParserInfo info = relationalExpression();
	if (info.er != none)
	{
		return info;
	}
	// {( & | | ) relationalExpression }
	Token next_token = PeekNextToken();
	char opp[128];
	strcpy(opp, next_token.lx);
	while ((strcmp(next_token.lx, "&") *
			strcmp(next_token.lx, "|")) == 0)
	{
		// eat the token
		GetNextToken();
		// arithmeticExpression
		info = relationalExpression();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
		if (parsedOnce)
		{
			if (strcmp(opp, "&") == 0)
				fprintf(outputFile, "and\n");
			if (strcmp(opp, "|") == 0)
				fprintf(outputFile, "or\n");
		}
	}
	// no error encountered
	return InfoNoError;
}
// relationalExpression→ arithmeticExpression {( = | > | < ) arithmeticExpression }
ParserInfo relationalExpression()
{
	// arithmeticExpression
	ParserInfo info = arithmeticExpression();
	if (info.er != none)
		return info;
	// {( = | > | < ) arithmeticExpression }
	Token next_token = PeekNextToken();
	while ((strcmp(next_token.lx, "=") *
			strcmp(next_token.lx, "<") *
			strcmp(next_token.lx, ">")) == 0)
	{
		char operator[128];
		strcpy(operator, next_token.lx);
		int parsedOnce = getProgramTable()->parsedOnce;
		FILE *outputFile = getOutputFile();
		// eat the token
		GetNextToken();
		// arithmeticExpression
		info = arithmeticExpression();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
		if (parsedOnce)
		{
			if (strcmp(operator, "<") == 0)
				fprintf(outputFile, "lt\n");
			else if (strcmp(operator, ">") == 0)
				fprintf(outputFile, "gt\n");
			else if (strcmp(operator, "=") == 0)
				fprintf(outputFile, "eq\n");
		}
	}
	// no error encountered
	return InfoNoError;
}
// arithmeticExpression → term {( + | - ) term }
ParserInfo arithmeticExpression()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	FILE *outputFile = getOutputFile();
	// term
	ParserInfo info = term();
	if (info.er != none)
		return info;
	// {( + | - ) term }
	Token next_token = PeekNextToken();
	char opp[128];
	strcpy(opp, next_token.lx);
	while ((strcmp(next_token.lx, "+") *
			strcmp(next_token.lx, "-")) == 0)
	{
		// eat the token
		GetNextToken();
		// term
		info = term();
		if (info.er != none)
			return info;
		if (parsedOnce)
		{
			if (strcmp(opp, "+") == 0)
				fprintf(outputFile, "add\n");
			else
				fprintf(outputFile, "sub\n");
		}
		next_token = PeekNextToken();
	}
	// no error encountered

	return InfoNoError;
}
// term → factor {( * | / ) factor }
ParserInfo term()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	FILE *outputFile = getOutputFile();
	// factor
	ParserInfo info = factor();
	if (info.er != none)
		return info;
	// {( * | / ) factor }
	Token next_token = PeekNextToken();
	char opp[128];
	strcpy(opp, next_token.lx);
	while ((strcmp(next_token.lx, "*") *
			strcmp(next_token.lx, "/")) == 0)
	{
		// eat the token
		GetNextToken();
		// factor
		info = factor();
		if (info.er != none)
			return info;
		if (parsedOnce)
		{
			if (strcmp(opp, "*") == 0)
				fprintf(outputFile, "call Math.multiply 2\n");
			else
				fprintf(outputFile, "call Math.divide 2\n");
		}
		next_token = PeekNextToken();
	}
	// if we have reached here there is a term parsed and no error
	return InfoNoError;
}
// factor →( - | ~ |ϵ) operand
ParserInfo factor()
{
	Token next_token = PeekNextToken();
	// - or ~
	if ((strcmp(next_token.lx, "-") *
		 strcmp(next_token.lx, "~")) == 0)
	{

		// eat the token before checking the operand
		GetNextToken();
	}
	// ϵ (no preceding token)
	// operand
	ParserInfo info = operand();
	FILE *outputFile = getOutputFile();
	int parsedOnce = getProgramTable()->parsedOnce;
	if (parsedOnce)
	{
		if (strcmp(next_token.lx, "~") == 0)
			fprintf(outputFile, "not\n");
		if (strcmp(next_token.lx, "-") == 0)
			fprintf(outputFile, "neg\n");
	}
	return info;
}
// dotIdentifier → .identifier
ParserInfo dotIdentifier()
{
	Token next_token = GetNextToken();
	// .
	if (strcmp(next_token.lx, ".") != 0)
		return (ParserInfo){syntaxError, next_token};
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	// check if the identifier exists and return this result
	return InfoNoError; // isVarInScope(&next_token);
}
// wrappedExpressionList → (expressionList)
ParserInfo wrappedExpressionList()
{
	// (
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
	{
		return (ParserInfo){openParenExpected, next_token};
	}
	// check for ) skip extra recursion
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, ")") == 0)
	{
		GetNextToken();
		return InfoNoError;
	}
	// expressionList
	ParserInfo info = expressionList();
	if (info.er != none)
		return info;
	// )
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
	}

	return InfoNoError;
}
// wrappedExpression → ( expression )
ParserInfo wrappedExpression()
{
	// (
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
	{
		return (ParserInfo){openParenExpected, next_token};
	}
	// check for ) skip extra recursion
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, ")") == 0)
	{
		GetNextToken();
		return InfoNoError;
	}
	// expression
	ParserInfo info = expression();
	if (info.er != none)
		return info;
	// )
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
	}
	return InfoNoError;
}
// operand → integerConstant | identifier [.identifier][[ expression ]|( expressionList ) ] | ( expression ) | stringLiteral | true | false | null | this
ParserInfo operand()
{
	int parsedOnce = getProgramTable()->parsedOnce;
	FILE *outputFile = getOutputFile();
	ParserInfo info;
	Token next_token = GetNextToken();
	// integerConstant
	if (next_token.tp == INT)
	{
		if (parsedOnce)
			fprintf(outputFile, "push constant %s\n", next_token.lx);
		return InfoNoError;
	}
	// stringLiteral
	else if (next_token.tp == STRING)
	{
		if (parsedOnce)
		{
			int stringLength = strlen(next_token.lx);
			fprintf(outputFile, "push constant %d\ncall String.new 1\n", stringLength);
			for (int i = 0; i < stringLength; i++)
			{
				fprintf(outputFile, "push constant %d\ncall String.appendChar 2\n", next_token.lx[i]);
			}
		}
		return InfoNoError;
	}
	// true | false | null | this
	else if (next_token.tp == RESWORD)
	{
		// true
		if (strcmp(next_token.lx, "true") == 0)
		{
			if (parsedOnce)
				fprintf(outputFile, "push constant 0\nnot\n");
			return InfoNoError;
		}
		// false
		else if (strcmp(next_token.lx, "false") == 0)
		{
			if (parsedOnce)
				fprintf(outputFile, "push constant 0\n");
			return InfoNoError;
		}
		// null
		else if (strcmp(next_token.lx, "null") == 0)
		{
			if (parsedOnce)
				fprintf(outputFile, "push constant 0\n");
			return InfoNoError;
		}
		// this
		else if (strcmp(next_token.lx, "this") == 0)
		{
			if (parsedOnce)
				fprintf(outputFile, "push pointer 0\n");
			return InfoNoError;
		}
		else
			return (ParserInfo){syntaxError, next_token}; // check this is correct
	}
	// ( expression )
	else if (strcmp(next_token.lx, "(") == 0)
	{
		// check for ) skip extra recursion
		next_token = PeekNextToken();
		if (strcmp(next_token.lx, ")") == 0)
		{
			GetNextToken();
			return InfoNoError;
		}
		// expression
		ParserInfo info = expression();
		if (info.er != none)
			return info;
		// )
		next_token = GetNextToken();
		if (strcmp(next_token.lx, ")") != 0)
		{
			return (ParserInfo){closeParenExpected, next_token};
		}
	}
	// identifier [ .identifier ][ [expression] | ( expressionList ) ]
	else if (next_token.tp == ID)
	{
		Token first_token = next_token;

		TokenContext *firstTokenContext = getTokenContext(&first_token);
		Token second_token;

		// check if the identifier exists
		// ParserInfo info = isVarInScope(&next_token);
		// if (info.er != none)
		//	return info;

		next_token = PeekNextToken();
		int dotId = 0;
		// [ .identifier ]
		if (strcmp(next_token.lx, ".") == 0)
		{
			dotId = 1;
			// info = dotIdentifier();
			// if (info.er != none)
			//	return info;
			next_token = GetNextToken();
			// .
			if (strcmp(next_token.lx, ".") != 0)
				return (ParserInfo){syntaxError, next_token};
			// identifier
			next_token = GetNextToken();
			second_token = next_token;
			if (next_token.tp != ID)
				return (ParserInfo){idExpected, next_token};
			// check if the first identifier is a class that has been parsed
			// and then check if the current identifier exists in that scope
			if (parsedOnce)
			{
				info = isVarInScope(&first_token);
				if (info.er != none)
				{
					ClassTable *table = getMatchingClass(&first_token);
					if (table == 0)
						return (ParserInfo){undecIdentifier, first_token};
					info = isSubInClass(&next_token, table);
					if (info.er != none)
						return info;
				}
				else
				{
					info = isCallValid(&first_token, &next_token);
					if (info.er != none)
						return info;
				}
			}
		}
		else
		{
			// check if the given identifier is
			info = isVarInScope(&first_token);
			if (info.er != none)
				return info;
		}
		// [ [ expression ] | ( expressionList ) ] = [ expressionList ]
		// means 0 or 1 of [expression] or (expressionList)
		next_token = PeekNextToken();
		if (strcmp(next_token.lx, "(") == 0)
		{
			// (
			GetNextToken();
			// we need to know how many arguments,
			int argCount = 0;
			int isClass = (unsigned long)getMatchingClass(&first_token);
			if (!isClass && dotId && parsedOnce)
			{
				argCount++;
				if (strcmp(firstTokenContext->kind, "argument") == 0)
					fprintf(outputFile, "push argument 1\n");
				else if (strcmp(firstTokenContext->kind, "var") == 0)
				{
					fprintf(outputFile, "push local %d\n", firstTokenContext->kindIndex);
				}
				else
				{
					if (strcmp(firstTokenContext->kind, "static") == 0)
						fprintf(outputFile, "push static %d\n", firstTokenContext->kindIndex);
					else
						fprintf(outputFile, "push this %d\n", firstTokenContext->kindIndex);
				}
			}
			// check for ) skip extra recursion
			next_token = PeekNextToken();
			if (strcmp(next_token.lx, ")") == 0)
			{
				GetNextToken();
			}
			else
			{
				// parse function arguments
				argCount++;
				// expressionList
				ParserInfo info = expression();
				if (info.er != none)
					return info;
				// {, expression }
				while (strcmp(PeekNextToken().lx, ",") == 0)
				{
					argCount++;
					// eat the ","
					GetNextToken();
					// expression
					ParserInfo info = expression();
					if (info.er != none)
						return info;
				}
				// )
				next_token = GetNextToken();
				if (strcmp(next_token.lx, ")") != 0)
				{
					return (ParserInfo){closeParenExpected, next_token};
				}
			}

			if (parsedOnce)
			{
				// you need to call a function.
				ClassTable *classTable = (ClassTable *)getScopeClass();
				if (!dotId)
				{
					fprintf(outputFile, "call %s.%s %d\n", classTable->name, first_token.lx, argCount);
				}
				else
				{
					if (!isClass)
						fprintf(outputFile, "call %s.%s %d\n", firstTokenContext->type, second_token.lx, argCount);
					else
						fprintf(outputFile, "call %s.%s %d\n", first_token.lx, second_token.lx, argCount);
				}
			}
		}
		else if (strcmp(next_token.lx, "[") == 0)
		{
			// [expression]
			// eat the [
			GetNextToken();
			// expression
			info = expression();
			if (info.er != none)
				return info;
			// ]
			next_token = GetNextToken();
			if (strcmp(next_token.lx, "]") != 0)
			{
				return (ParserInfo){closeBracketExpected, next_token};
			}
			if (parsedOnce)
			{
				if (firstTokenContext->isLocal)
					fprintf(outputFile, "push local %d\nadd\npop pointer 1\npush that 0\n", firstTokenContext->kindIndex);
				else
				{
					if (strcmp(firstTokenContext->kind, "static") == 0)
						fprintf(outputFile, "push static %d\n", firstTokenContext->kindIndex);
					else
						fprintf(outputFile, "push this %d\nadd\n", firstTokenContext->kindIndex);
				}
			}
		}
		else
		{
			// single identifier
			if (parsedOnce)
			{
				if (firstTokenContext->isLocal)
				{
					if (strcmp(firstTokenContext->kind, "var") == 0)
						fprintf(outputFile, "push local %d\n", firstTokenContext->kindIndex);
					else
						fprintf(outputFile, "push %s %d\n", firstTokenContext->kind, firstTokenContext->kindIndex);
				}
				else
				{
					if (strcmp(firstTokenContext->kind, "static") == 0)
						fprintf(outputFile, "push static %d\n", firstTokenContext->kindIndex);
					else
						fprintf(outputFile, "push this %d\n", firstTokenContext->kindIndex);
				}
			}
		}
		free(firstTokenContext);
	}
	else
		return (ParserInfo){syntaxError, next_token};
	// successfully parsed, return no error
	return InfoNoError;
}
// you can declare prototypes of parser functions below

int InitParser(char *file_name)
{
	// initialise the lexer
	int lexerError = InitLexer(file_name);
	if (lexerError == 0)
	{
		printf("Lexer encountered an error\n");
		return lexerError;
	}
	// set info no error
	Token t = PeekNextToken();
	InfoNoError = (ParserInfo){none, t};
	return 1;
}

ParserInfo Parse()
{
	ParserInfo pi;
	// First check for lexer errror
	// Check for a at tokenisation stage
	Token first_token = PeekNextToken();
	if (first_token.tp == ERR)
	{
		pi.er = lexerErr;
		pi.tk = first_token;
		return pi;
	}
	// now parse the class
	pi = class();
	switch (pi.er)
	{
	case lexerErr:
		printf("Lexer Error at: ");
		// printToken(pi.tk);
		break;
	}

	return pi;
}

int StopParser()
{
	StopLexer();
	return 1;
}
/*
#ifndef TEST_PARSER
int main()
{
InitParser("./testfiles/NewLineInStr2.jack");
ParserInfo info = Parse();
printf("(%d,%s) near line %d\n", info.er, info.tk.lx, info.tk.ln);
printf("End\n");
return 1;
}
#endif*/
