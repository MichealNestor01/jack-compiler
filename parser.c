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
ParserInfo type();
// subroutineDeclar→( constructor|funtoin|method)( type | void ) identifier( paramList ) subroutineBody
ParserInfo subroutineDeclar();
// paramList→type identifier {, type identifier }|ϵ
ParserInfo paramList();
// subroutineBody→ { { statement } }
ParserInfo subroutineBody();
// Statement Grammar:
// statement → varDeclarStatement | letStatemnt | ifStatement | whileStatement | doStatement | returnStatemnt
ParserInfo statement();
// varDeclarStatement→var type identifier {, identifier };
ParserInfo varDeclarStatement();
// letStatement → let identifier [ [ expression ] ] = expression ;
ParserInfo letStatement();
// ifStatement→if ( expression ) { { statement } } [ else { { statement } } ]
ParserInfo ifStatement();
// whileStatement → while ( expression ) { { statement } }
ParserInfo whileStatement();
// doStatement → do subroutineCall ;
ParserInfo doStatement();
// subroutineCall → identifier [.identifier] ( expressionList )
ParserInfo subroutineCall();
// expressoinList → expression {, expression }|ϵ
ParserInfo expressionList();
// returnStatement → return [ expression ];
ParserInfo returnStatment();
// Expressions Grammar:
// expresion→ relationalEpression {( & | | ) relationalExpression }
ParserInfo expression();
// relationalExpression→ ArithmeticExpression {( = | > | < ) ArithmeticExpression )
ParserInfo relationalExpression();
// ArithmeticExpression → term {( + | - ) term }
ParserInfo arithmeticExpression();
// term → factor {( * | / ) factor }
ParserInfo term()
{
	// factor
	ParserInfo info = factor();
	if (info.er != none)
		return info;
	// {( * | / ) factor }
	Token next_token = PeekNextToken();
	while ((strcmp(next_token.lx, "*") * strcmp(next_token.lx, "/")) == 0)
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
	if (strcmp(next_token.lx, "-") == 0 || strcmp(next_token.lx, "~" == 0))
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
	if (strcmp(next_token.lx, ".") == 0)
		return InfoNoError;
	else
		return (ParserInfo){syntaxError, next_token};
	// identifier
	next_token = GetNextToken();
	if (next_token.tp == ID)
		return InfoNoError;
	else
		return (ParserInfo){idExpected, next_token};
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
