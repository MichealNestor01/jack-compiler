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
	// class
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "class") != 0)
		return (ParserInfo){classExpected, next_token};
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	// Check this class has not already been created
	ParserInfo info = addTokenToProgramTable(&next_token);
	if (info.er != none)
		return info;
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
	// validate that this var has not already been defined
	info = addVarTokenToClassTable(&next_token, typeString, kindString);
	if (info.er != none)
		return info;
	// {, identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ,
		GetNextToken();
		// identifier
		Token next_token = GetNextToken();
		if (next_token.tp != ID)
			return (ParserInfo){idExpected, next_token};
		// validate that this var has not already been defined
		info = addVarTokenToClassTable(&next_token, typeString, kindString);
		if (info.er != none)
			return info;
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
	// validate that this subroutine has not already been defined
	info = addSubTokenToClassTable(&next_token, typeString, kindString);
	if (info.er != none)
		return info;
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
		info = addTokenToSubroutineTable(&next_token, typeString, "argument");
		if (info.er != none)
			return info;
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
			info = addTokenToSubroutineTable(&next_token, typeString, "argument");
			if (info.er != none)
				return info;
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
	info = addTokenToSubroutineTable(&next_token, typeString, "var");
	if (info.er != none)
		return info;
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
	// [ [identifier] ]
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "[") == 0)
	{
		// eat the "["
		GetNextToken();
		// identifier
		next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
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
	ParserInfo info = expression();
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
	// identifier
	Token next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	next_token = PeekNextToken();
	// [ .identifier ]
	if (strcmp(next_token.lx, ".") == 0)
	{
		ParserInfo info = dotIdentifier();
		if (info.er != none)
			return info;
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
		return operand();
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

	return InfoNoError;
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
		next_token = PeekNextToken();
		// [ .identifier ]
		if (strcmp(next_token.lx, ".") == 0)
		{
			info = dotIdentifier();
			if (info.er != none)
				return info;
		}

		// [ [ expression ] | ( expressionList ) ] = [ expressionList ]
		// means 0 or 1 of [expression] or (expressionList)
		next_token = PeekNextToken();
		if (strcmp(next_token.lx, "(") == 0)
		{
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
			ParserInfo info = expression();
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
