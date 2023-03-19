#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

// error function
void error(char *s)
{
	printf("Error: %s", s);
	exit(1);
}

// no error parser info
ParserInfo InfoNoError = {none, NULL};

// you need to implemenent these grammars:
// Class Grammar:
// class→class identifier { { memeberDeclar } }
ParserInfo class();
// memberDeclar→classVarDeclar | subroutineDeclar
ParserInfo memberDeclar();
// classVarDeclar→(static|field) type identifier {, identifier};
ParserInfo classVarDeclar();
// type→int|char|boolean|identifier
ParserInfo type()
{
	Token next_token = GetNextToken();
	// int|char|boolean|identifier
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
	// (constructor|function|method)
	Token next_token = GetNextToken();
	if ((strcmp(next_token.lx, "constructor") *
		 strcmp(next_token.lx, "function") *
		 strcmp(next_token.lx, "method")) != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// ( type | void )
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "void") == 0)
	{
		// eat void
		GetNextToken();
	}
	else
	{
		// type
		ParserInfo info = type();
		if (info.er != none)
		{
			return info;
		}
	}
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	// (
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
	{
		return (ParserInfo){openParenExpected, next_token};
	}
	// paramList
	ParserInfo info = paramList();
	if (info.er != none)
		return info;
	// )
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
	}
	// subroutineBody
	ParserInfo info = subroutineBody();
	if (info.er != none)
		return info;
	return InfoNoError;
}
// paramList→type identifier {, type identifier }|ϵ
ParserInfo paramList()
{
	// type
	ParserInfo info = type();
	if (info.er != none)
	{
		return info;
	}
	// indentifier
	Token next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	// {, type identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ,
		GetNextToken();
		// type
		ParserInfo info = type();
		if (info.er != none)
		{
			return info;
		}
		// identifier
		Token next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
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
	// statement
	while (statement().er == none)
		;
	// }
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "{") != 0)
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
	// type
	Token next_token = GetNextToken();
	if (strcmp(next_token.lx, "type") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
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
		return info;
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
	ParserInfo info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
	}
	// [ else { { statement } } ]
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, "else") != 0)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// { { statement } }
	ParserInfo info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
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
	// {
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "{") != 0)
	{
		return (ParserInfo){openBraceExpected, next_token};
	}
	// {statement}
	ParserInfo info = wrappedZeroOrMoreStatements();
	if (info.er != none)
	{
		return info;
	}
	// {
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "}") != 0)
	{
		return (ParserInfo){closeBraceExpected, next_token};
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
	ParserInfo info = expressionList();
	if (info.er != none)
		return info;
	// ;
	Token next_token = GetNextToken();
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
	// (
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
	{
		return (ParserInfo){openParenExpected, next_token};
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
	// relationalExpression
	ParserInfo info = relationalExpression();
	if (info.er != none)
		return info;
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
// operand → integerConstant | identifier [.identifier][[ expression ]|( expressionList ) ] | ( expression ) | stringLiteral | true | false | null | this
ParserInfo operand()
{
	Token next_token = GetNextToken();
	// integerConstant
	if (next_token.tp == INT)
		return InfoNoError;
	// stringLiteral
	else if (next_token.tp == STRING)
		return InfoNoError;
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
	else
	{
		// identifier [ .identifier ][ [expression] | ( expressionList ) ]
		// identifier
		if (next_token.tp == ID)
		{
			// [ .identifier ][ [expression] | ( expressionList ) ]
			Token next_next_token = PeekNextToken();
			// [ .identifier ]
			if (strcmp(next_next_token.lx, ".") == 0)
			{
				ParserInfo info = dotIdentifier();
				if (info.er != none)
					return info;
				// update next_next_token for the next part
				next_next_token = PeekNextToken();
			}
			// [ [expression] | ( expressionList ) ]
			// [expression]
			// [ expression ]

			// Gonna need to have a think about how the or works here

			// To do this you will need to implement expression
		}
		else
			(ParserInfo){idExpected, next_token};
	}
}
// you can declare prototypes of parser functions below

int InitParser(char *file_name)
{

	return 1;
}

ParserInfo Parse()
{
	ParserInfo pi;

	// implement the function

	pi.er = none;
	return pi;
}

int StopParser()
{
	return 1;
}

#ifndef TEST_PARSER
int main()
{

	return 1;
}
#endif
