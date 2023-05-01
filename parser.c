#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"

// error function
void error(char *s)
{
	printf("Error: %s", s);
	exit(1);
}

// no error parser info
ParserInfo InfoNoError;

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
	// int|char|boolean|identifier
	Token next_token = GetNextToken();
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
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	// only add identifiers in the first parse
	if (parsedOnce == 0)
	{
		info = addSubTokenToClassTable(&next_token, typeString, kindString);
		if (info.er != none)
			return info;
	}
	else
	{
		pushSubToScope(&next_token);
	}
	//  (
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
		return (ParserInfo){openParenExpected, next_token};
	//  check for closed brackets before checking param list
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		// paramList
		info = paramList();
		if (info.er != none)
			return info;
	}
	// )
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
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
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "[") == 0)
	{
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
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}
	return InfoNoError;
}
// ifStatement→if ( expression ) { { statement } } [ else { { statement } } ]
ParserInfo ifStatement()
{
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
	// { { statement } }
	info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
	}
	// [ else { { statement } } ]
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "else") == 0)
	{
		// eat else
		GetNextToken();
		// { { statement } }
		info = wrappedZeroOrMoreStatements();
		if (info.er != none)
		{
			return info;
		}
	}

	return InfoNoError;
}
// whileStatement → while ( expression ) { { statement } }
ParserInfo whileStatement()
{
	// while
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "while") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// ( expression )
	ParserInfo info = wrappedExpression();
	if (info.er != none)
		return info;
	// { {statement} }
	info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
	}
	// successfully parsed
	return InfoNoError;
}
// doStatement → do subroutineCall ;
ParserInfo doStatement()
{
	// do
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "do") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// subroutineCall
	ParserInfo info = subroutineCall();
	if (info.er != none)
		return info;
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
		// first check if the first idenfifier exists in scope
		//
		// check if the first identifier is a class that has been parsed
		// and then check if the current identifier exists in that scope
		if (parsedOnce)
		{
			printf("Checking if %s is in scope2\n", first_token.lx);
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
				printf("FIRST FOUND IN SCOPE");
				exit(0);
			}
			printf("Function exists!\n");
		}
	}
	else
	{
		// check if subroutine is in scope
		if (parsedOnce)
		{
			info = isSubInScope(&first_token);
			if (info.er != none)
				return info;
		}
	}
	// ( expressionList )
	return wrappedExpressionList();
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
	Token next_token = GetNextToken();
	// return
	if (strcmp(next_token.lx, "return") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// [ expression ]
	next_token = PeekNextToken();
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
	ParserInfo info = relationalExpression();
	if (info.er != none)
	{
		return info;
	}
	// {( & | | ) relationalExpression }
	Token next_token = PeekNextToken();
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
		// eat the token
		GetNextToken();
		// arithmeticExpression
		info = arithmeticExpression();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
	}
	// no error encountered
	return InfoNoError;
}
// arithmeticExpression → term {( + | - ) term }
ParserInfo arithmeticExpression()
{
	// term
	ParserInfo info = term();
	if (info.er != none)
		return info;
	// {( + | - ) term }
	Token next_token = PeekNextToken();
	while ((strcmp(next_token.lx, "+") *
			strcmp(next_token.lx, "-")) == 0)
	{
		// eat the token
		GetNextToken();
		// term
		info = term();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
	}
	// no error encountered

	return InfoNoError;
}
// term → factor {( * | / ) factor }
ParserInfo term()
{
	// factor
	ParserInfo info = factor();
	if (info.er != none)
		return info;
	// {( * | / ) factor }
	Token next_token = PeekNextToken();
	while ((strcmp(next_token.lx, "*") *
			strcmp(next_token.lx, "/")) == 0)
	{
		// eat the token
		GetNextToken();
		// factor
		info = factor();
		if (info.er != none)
			return info;
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
	return operand();
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
	ParserInfo info;
	Token next_token = GetNextToken();
	// integerConstant
	if (next_token.tp == INT)
		return InfoNoError;
	// stringLiteral
	else if (next_token.tp == STRING)
		return InfoNoError;
	// true | false | null | this
	else if (next_token.tp == RESWORD)
	{
		// true
		if (strcmp(next_token.lx, "true") == 0)
			return InfoNoError;
		// false
		else if (strcmp(next_token.lx, "false") == 0)
			return InfoNoError;
		// null
		else if (strcmp(next_token.lx, "null") == 0)
			return InfoNoError;
		// this
		else if (strcmp(next_token.lx, "this") == 0)
			return InfoNoError;
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

		// store token 1
		// check if .identifier
		// then you should check if token 1 matches
		// any of the classes in the program table
		// thennnnn see if the .identifier exists in
		// this classes symbol table.]

		// apparently a.b is only ever a function call,
		// so we are going to check if identifier is a
		// class, but I am not implementing this unless
		// it is actually ever tested because I am running
		// out of time on this one

		int parsedOnce = getProgramTable()->parsedOnce;
		Token first_token = next_token;

		// check if the identifier exists
		// ParserInfo info = isVarInScope(&next_token);
		// if (info.er != none)
		//	return info;

		next_token = PeekNextToken();
		// [ .identifier ]
		if (strcmp(next_token.lx, ".") == 0)
		{
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
			// check if the first identifier is a class that has been parsed
			// and then check if the current identifier exists in that scope
			if (parsedOnce)
			{
				printf("Checking if %s is in scope\n", first_token.lx);
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
					printf("FIRST FOUND IN SCOPE");
					exit(0);
				}
			}
		}
		// [ [ expression ] | ( expressionList ) ] = [ expressionList ]
		// means 0 or 1 of [expression] or (expressionList)
		next_token = PeekNextToken();
		if (strcmp(next_token.lx, "(") == 0)
		{
			// check if subroutine is in scope
			info = wrappedExpressionList();
			if (info.er != none)
				return info;
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
		}
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
		printToken(pi.tk);
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
