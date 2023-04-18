#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// #include "lexer.h"
#include "parser.h"

// error function
void error(char *s)
{
	printf("Error: %s", s);
	exit(1);
}

// show debug statements
int SHOWDEBUG = 1;
int DEPTH = 0;

// no error parser info
ParserInfo InfoNoError;

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

// you need to implemenent these grammars:
// Class Grammar:
// class→class identifier { { memeberDeclar } }
ParserInfo class()
{
	DEPTH++;
	int a = DEPTH;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING CLASS {%s}\n", DEPTH, t.lx);
	// class
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "class") != 0)
	{
		return (ParserInfo){classExpected, next_token};
	}
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// {
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "{") != 0)
	{
		return (ParserInfo){openBraceExpected, next_token};
	}
	// {memberdeDeclar}
	ParserInfo info;
	next_token = PeekNextToken();
	while ((strcmp(next_token.lx, "static") *
			strcmp(next_token.lx, "field") *
			strcmp(next_token.lx, "constructor") *
			strcmp(next_token.lx, "function") *
			strcmp(next_token.lx, "method")) == 0)
	{
		if (SHOWDEBUG)
			printf("\tCurrent token: {%s}\n", next_token.lx);
		info = memberDeclar();
		if (info.er != none)
			return info;
		next_token = PeekNextToken();
		if (SHOWDEBUG)
			printf("\tToken for next memberDeclar: {%s}\n", next_token.lx);
	}
	// }
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "}") != 0)
	{
		return (ParserInfo){closeBraceExpected, next_token};
	}
	if (SHOWDEBUG)
		printf("Class Parsed Successfully\n");
	return InfoNoError;
}
// memberDeclar→classVarDeclar | subroutineDeclar
ParserInfo memberDeclar()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING memberDeclar {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING classVarDecalr {%s}\n", DEPTH, t.lx);
	// static|field
	Token next_token = GetNextToken();
	if ((strcmp(next_token.lx, "static") *
		 strcmp(next_token.lx, "field")) != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// type
	ParserInfo info = type();
	if (info.er != none)
		return info;
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
		return (ParserInfo){syntaxError, next_token};
	// {, identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ,
		GetNextToken();
		// identifier
		Token next_token = GetNextToken();
		if (next_token.tp != ID)
			return (ParserInfo){idExpected, next_token};
	}
	// ;
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ";") != 0)
		return (ParserInfo){semicolonExpected, next_token};
	if (SHOWDEBUG)
		printf("\tclassVarDeclar Parsed Successfully\n");
	return InfoNoError;
}
// type→int|char|boolean|identifier
ParserInfo type()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING type {%s}\n", DEPTH, t.lx);
	// int|char|boolean|identifier
	Token next_token = GetNextToken();
	if (next_token.tp == ID ||
		(strcmp(next_token.lx, "int") *
		 strcmp(next_token.lx, "char") *
		 strcmp(next_token.lx, "boolean")) == 0)
	{
		// if (SHOWDEBUG) printf("type Parsed Successfully");

		return InfoNoError;
	}
	if (SHOWDEBUG)
		printf("type not parsed: (%s)\n", next_token.lx);
	// should this be idExpected or Syntax error?
	return (ParserInfo){idExpected, next_token};
}
// subroutineDeclar→( constructor|funtoin|method)( type | void ) identifier( paramList ) subroutineBody
ParserInfo subroutineDeclar()
{
	DEPTH++;
	int a = DEPTH;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING subroutineDeclar {%s}\n", DEPTH, t.lx);
	ParserInfo info;
	// (constructor|function|method)
	Token next_token = GetNextToken();
	if ((strcmp(next_token.lx, "constructor") *
		 strcmp(next_token.lx, "function") *
		 strcmp(next_token.lx, "method")) != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND KEYWORD: (%s)\n", next_token.lx);
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
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND TYPE OR VOID: (%s)\n", next_token.lx);
	//  identifier
	next_token = GetNextToken();
	if (SHOWDEBUG)
		printf("\tNEED IDENTIFIER {%s} {%d}\n", next_token.lx, next_token.tp == ID);
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND IDENTIFIER: (%s)\n", next_token.lx);
	//  (
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
		return (ParserInfo){openParenExpected, next_token};
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND OPEN PAREN: (%s)\n", next_token.lx);
	//  check for closed brackets before checking param list
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		// paramList
		if (SHOWDEBUG)
			printf("\tChecking paramlist\n");
		info = paramList();
		if (info.er != none)
			return info;
	}
	////if (SHOWDEBUG) printf("\tSUBDEC: FOUND PARAMLIST\n");
	// )
	if (SHOWDEBUG)
		printf("\tChecking close paren\n");
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
	}
	////if (SHOWDEBUG) printf("\tSUBDEC: FOUND CLOSED PAREN: (%s)\n", next_token.lx);
	// subroutineBody
	info = subroutineBody();
	if (info.er != none)
	{
		if (SHOWDEBUG)
			printf("\t(%d) ERROR CAUGHT IN SUBROUTINE BODY [%s]\n", a, info.tk.lx);
		return info;
	}
	// if (SHOWDEBUG) printf("subroutine declare Parsed Successfully");

	return InfoNoError;
}
// paramList→(type identifier {, type identifier })|ϵ
ParserInfo paramList()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING paramList {%s}\n", DEPTH, t.lx);
	Token next_token = PeekNextToken();
	if (next_token.tp == ID ||
		(strcmp(next_token.lx, "int") *
		 strcmp(next_token.lx, "char") *
		 strcmp(next_token.lx, "boolean")) == 0)
	{
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
		//  {, type identifier }
		while (strcmp(PeekNextToken().lx, ",") == 0)
		{
			//  eat the ,
			GetNextToken();
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING wrappedZeroOrMoreStatements {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING statement {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING varDeclarStatement {%s}\n", DEPTH, t.lx);
	// var
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "var") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
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
	DEPTH++;
	int a = DEPTH;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING letStatement {%s}\n", DEPTH, t.lx);
	// let
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "let") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	if (SHOWDEBUG)
		printf("\tFound let\n");
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	if (SHOWDEBUG)
		printf("\tFound identifier\n");
	// [ [identifier] ]
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "[") == 0)
	{
		if (SHOWDEBUG)
			printf("\tFound [\n");
		// eat the "["
		GetNextToken();
		// identifier
		next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
		}
		if (SHOWDEBUG)
			printf("\tFound identifier\n");
		// "]"
		next_token = GetNextToken();
		if (strcmp(next_token.lx, "]") != 0)
		{
			return (ParserInfo){syntaxError, next_token};
		}
		if (SHOWDEBUG)
			printf("\tFound ]\n");
	}
	// =
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "=") != 0)
	{
		return (ParserInfo){equalExpected, next_token};
	}
	if (SHOWDEBUG)
		printf("\tFound =\n");
	// expression
	ParserInfo info = expression();
	if (info.er != none)
	{
		if (SHOWDEBUG)
			printf("\t(%d) error caught in expression [%s]\n", a, info.tk.lx);
		return info;
	}
	if (SHOWDEBUG)
		printf("\tFound expression\n");
	// ;
	next_token = GetNextToken();
	if (SHOWDEBUG)
		printf("\tNextToken: {%s} near {%d}\n", next_token.lx, next_token.ln);
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}
	if (SHOWDEBUG)
		printf("\tFound ;\n");
	if (SHOWDEBUG)
		printf("PARSED letStatement\n");

	return InfoNoError;
}
// ifStatement→if ( expression ) { { statement } } [ else { { statement } } ]
ParserInfo ifStatement()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING ifStatement {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING whileStatement {%s}\n", DEPTH, t.lx);
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
	if (SHOWDEBUG)
		printf("\tParsed while statement\n");

	return InfoNoError;
}
// doStatement → do subroutineCall ;
ParserInfo doStatement()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING doStatement {%s}\n", DEPTH, t.lx);
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
	if (SHOWDEBUG)
		printf("\tDO STATEMENT - LOOKING FOR ; {%s}", next_token);
	if (strcmp(next_token.lx, ";") != 0)
	{
		return (ParserInfo){semicolonExpected, next_token};
	}

	return InfoNoError;
};
// subroutineCall → identifier [.identifier] ( expressionList )
ParserInfo subroutineCall()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING subroutineCall {%s}\n", DEPTH, t.lx);
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
	printf("\tCalling wrapped expresison list {%s}\n", next_token.lx);
	return wrappedExpressionList();
}
// expressoinList → expression {, expression }|ϵ
ParserInfo expressionList()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING expressionList {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING returnStatement {%s}\n", DEPTH, t.lx);
	Token next_token = GetNextToken();
	// return
	if (strcmp(next_token.lx, "return") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// [ expression ];
	next_token = PeekNextToken();
	// if the next token is a ;, then there was no expression
	// else check for an expression.
	if (strcmp(next_token.lx, ";") == 0)
	{
		GetNextToken();

		return InfoNoError;
	}
	// [ expression ]
	ParserInfo info = expression();
	if (info.er != none)
		return info;
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
	DEPTH++;
	int a = DEPTH;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING expression {%s}\n", DEPTH, t.lx);
	// relationalExpression
	ParserInfo info = relationalExpression();
	if (info.er != none)
	{
		if (SHOWDEBUG)
			printf("\t(%d) Bad REXPRESS found in EXPRESS [%s]\n", a, info.tk.lx);
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
	if (SHOWDEBUG)
		printf("PARSED expression\n");
	// no error encountered

	return InfoNoError;
}
// relationalExpression→ arithmeticExpression {( = | > | < ) arithmeticExpression }
ParserInfo relationalExpression()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING relationalExpression {%s}\n", DEPTH, t.lx);
	// arithmeticExpression
	ParserInfo info = arithmeticExpression();
	if (info.er != none)
	{
		if (SHOWDEBUG)
			printf("\t(%d) BAD AExpress found in REXPRESS\n", DEPTH);
		return info;
	}
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
		{
			if (SHOWDEBUG)
				printf("\t(%d) Bad AExpress found in RExpress 2\n", DEPTH);
			return info;
		}
		next_token = PeekNextToken();
	}
	if (SHOWDEBUG)
		printf("PARSED relationalExpression\n");
	// no error encountered

	return InfoNoError;
}
// arithmeticExpression → term {( + | - ) term }
ParserInfo arithmeticExpression()
{
	DEPTH++;
	int a = DEPTH;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING arithmeticExpression {%s}\n", DEPTH, t.lx);
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
		{
			if (SHOWDEBUG)
				printf("\t(%d) Bad term found in AExpress [%s]\n", a, info.tk.lx);

			return info;
		}
		next_token = PeekNextToken();
	}
	if (SHOWDEBUG)
		printf("PARSED arithmeticExpression\n");
	// no error encountered

	return InfoNoError;
}
// term → factor {( * | / ) factor }
ParserInfo term()
{
	DEPTH++;
	int a = DEPTH;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING term {%s}\n", DEPTH, t.lx);
	// factor
	ParserInfo info = factor();
	if (info.er != none)
	{
		if (SHOWDEBUG)
			printf("\t(%d) Bad factor found in term [%s]\n", a, info.tk.lx);
		return info;
	}
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
	if (SHOWDEBUG)
		printf("PARSED term\n");
	// if we have reached here there is a term parsed and no error

	return InfoNoError;
}
// factor →( - | ~ |ϵ) operand
ParserInfo factor()
{
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING factor {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING dotIdentifier {%s}\n", DEPTH, t.lx);
	Token next_token = GetNextToken();
	// .
	if (strcmp(next_token.lx, ".") != 0)
		return (ParserInfo){syntaxError, next_token};
	if (SHOWDEBUG)
		printf("\tParsed Dot\n");
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
		return (ParserInfo){idExpected, next_token};
	if (SHOWDEBUG)
		printf("\tParsed Identifier\n");

	return InfoNoError;
}
// wrappedExpressionList → (expressionList)
ParserInfo wrappedExpressionList()
{
	DEPTH++;
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
	{
		printf("\tExpressionList returned error\n");
		return info;
	}
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING wrappedExpression {%s}\n", DEPTH, t.lx);
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
	DEPTH++;
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("(%d) PARSING operand {%s}\n", DEPTH, t.lx);
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
		if (SHOWDEBUG)
			printf("\tFound bracket\n");
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
		if (SHOWDEBUG)
			printf("\tFound identifier\n");
		next_token = PeekNextToken();
		if (SHOWDEBUG)
			printf("\tcurrent token: {%s}\n", next_token.lx);
		// [ .identifier ]
		if (strcmp(next_token.lx, ".") == 0)
		{
			info = dotIdentifier();
			if (info.er != none)
				return info;
			if (SHOWDEBUG)
				printf("\tFound [.identifier]\n");
		}
		else
		{
			if (SHOWDEBUG)
				printf("\tNo [.identifier]\n");
		}

		// [ [ expression ] | ( expressionList ) ] = [ expressionList ]
		// means 0 or 1 of [expression] or (expressionList)
		next_token = PeekNextToken();
		if (SHOWDEBUG)
			printf("\tcurrent token: {%s}\n", next_token.lx);
		if (strcmp(next_token.lx, "(") == 0)
		{
			info = wrappedExpressionList();
			if (info.er != none)
				return info;
		}
		else if (strcmp(next_token.lx, "[") == 0)
		{
			if (SHOWDEBUG)
				printf("\t(%d) Found [, looking for expression\n", DEPTH);
			int a = DEPTH;
			// [expression]
			// eat the [
			GetNextToken();
			// expression
			ParserInfo info = expression();
			if (info.er != none)
				return info;
			if (SHOWDEBUG)
				printf("\t(%d) Looking for close bracket\n", a);
			// ]
			next_token = GetNextToken();
			if (SHOWDEBUG)
				printf("\t(%d) current token: {%s}\n", a, next_token.lx);
			if (strcmp(next_token.lx, "]") != 0)
			{
				if (SHOWDEBUG)
					printf("\t(%d) Not found\n", a);
				return (ParserInfo){closeBracketExpected, next_token};
			}
		}
	}
	else
		(ParserInfo){syntaxError, next_token};
	// successfully parsed, return no error
	if (SHOWDEBUG)
		printf("PARSED operand\n");

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
	// initialise the lexer
	// do some other shites
	return 1;
}

ParserInfo Parse()
{
	ParserInfo pi;
	// implement the function
	// start by parsing the class
	pi = class();
	switch (pi.er)
	{
	case lexerErr:
		printf("Lexer Error at: ");
		printToken(pi.tk);
		break;
	}
	// pi.er = none;
	return pi;
}

int StopParser()
{
	StopLexer();
	return 1;
}

#ifndef TEST_PARSER
int main()
{
	InitParser("./testfiles/Ball.jack");
	ParserInfo info = Parse();
	printf("(%d,%s) near line %d\n", info.er, info.tk.lx, info.tk.ln);
	printf("End\n");
	return 1;
}
#endif
