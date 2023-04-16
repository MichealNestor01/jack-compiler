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
int SHOWDEBUG = 0;

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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING CLASS {%s}\n", t.lx);
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
	next_token = PeekNextToken();
	// static|field|constructor|function|method
	if ((strcmp(next_token.lx, "static") *
		 strcmp(next_token.lx, "field") *
		 strcmp(next_token.lx, "constructor") *
		 strcmp(next_token.lx, "function") *
		 strcmp(next_token.lx, "method")) == 0)
	{
		return memberDeclar();
	}
	// }
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "{") != 0)
	{
		return (ParserInfo){openBraceExpected, next_token};
	}
	if (SHOWDEBUG)
		printf("Class Parsed Successfully\n");
	return InfoNoError;
}
// memberDeclar→classVarDeclar | subroutineDeclar
ParserInfo memberDeclar()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING memberDeclar {%s}\n", t.lx);
	Token next_token = PeekNextToken();
	// try classVarDeclare
	// static|field
	if ((strcmp(next_token.lx, "static") *
		 strcmp(next_token.lx, "field")) == 0)
	{
		// if (SHOWDEBUG) if (SHOWDEBUG) printf("memberDeclar Parsed Successfully 1\n");
		return classVarDeclar();
	}
	// else try subroutineDeclare
	// constructor|function|method
	if ((strcmp(next_token.lx, "constructor") *
		 strcmp(next_token.lx, "function") *
		 strcmp(next_token.lx, "method")) == 0)
	{
		// if (SHOWDEBUG) printf("memberDeclar Parsed Successfully 2\n");
		return subroutineDeclar();
	}
	return (ParserInfo){syntaxError, next_token};
}
// classVarDeclar→(static|field) type identifier {, identifier};
ParserInfo classVarDeclar()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING classVarDecalr {%s}\n", t.lx);
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
	{
		return info;
	}
	// identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){syntaxError, next_token};
	}
	// {, identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// eat the ,
		GetNextToken();
		// identifier
		Token next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
		}
	}
	// if (SHOWDEBUG) printf("classVarDeclar Parsed Successfully");
	return InfoNoError;
}
// type→int|char|boolean|identifier
ParserInfo type()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING type {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING subroutineDeclar {%s}\n", t.lx);
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
		{
			return info;
		}
	}
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND TYPE OR VOID: (%s)\n", next_token.lx);
	//  identifier
	next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND IDENTIFIER: (%s)\n", next_token.lx);
	//  (
	next_token = GetNextToken();
	if (strcmp(next_token.lx, "(") != 0)
	{
		return (ParserInfo){openParenExpected, next_token};
	}
	// if (SHOWDEBUG) printf("\tSUBDEC: FOUND OPEN PAREN: (%s)\n", next_token.lx);
	//  check for closed brackets before checking param list
	next_token = PeekNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		// paramList
		info = paramList();
		if (info.er != none)
			return info;
	}
	////if (SHOWDEBUG) printf("\tSUBDEC: FOUND PARAMLIST\n");
	// )
	next_token = GetNextToken();
	if (strcmp(next_token.lx, ")") != 0)
	{
		return (ParserInfo){closeParenExpected, next_token};
	}
	////if (SHOWDEBUG) printf("\tSUBDEC: FOUND CLOSED PAREN: (%s)\n", next_token.lx);
	// subroutineBody
	info = subroutineBody();
	if (info.er != none)
		return info;
	// if (SHOWDEBUG) printf("subroutine declare Parsed Successfully");
	return InfoNoError;
}
// paramList→(type identifier {, type identifier })|ϵ
ParserInfo paramList()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING paramList {%s}\n", t.lx);
	// type
	ParserInfo info = type();
	if (info.er != none)
	{
		return info;
	}
	// if (SHOWDEBUG) printf("PARAM LIST: FOUND TYPE\n");
	//  indentifier
	Token next_token = GetNextToken();
	if (next_token.tp != ID)
	{
		return (ParserInfo){idExpected, next_token};
	}
	// if (SHOWDEBUG) printf("PARAM LIST: FOUND IDENTIFIER: (%s)\n", next_token.lx);
	//  {, type identifier }
	while (strcmp(PeekNextToken().lx, ",") == 0)
	{
		// if (SHOWDEBUG) printf("PARAM LIST: FOUND COMMA: (%s)\n", PeekNextToken().lx);
		//  eat the ,
		GetNextToken();

		// type
		ParserInfo info = type();
		if (info.er != none)
		{
			return info;
		}
		// if (SHOWDEBUG) printf("PARAM LIST: FOUND TYPE\n");
		//  identifier
		Token next_token = GetNextToken();
		if (next_token.tp != ID)
		{
			return (ParserInfo){idExpected, next_token};
		}
		// if (SHOWDEBUG) printf("PARAM LIST: FOUND IDENTIFIER: (%s)\n", next_token.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING wrappedZeroOrMoreStatements {%s}\n", t.lx);
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
	if (strcmp(next_token.lx, "}") != 0)
	{
		return (ParserInfo){closeBraceExpected, next_token};
	}
	return InfoNoError;
}
// statement → varDeclarStatement | letStatemnt | ifStatement | whileStatement | doStatement | returnStatemnt
ParserInfo statement()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING statement {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING varDeclarStatement {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING letStatement {%s}\n", t.lx);
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
		return info;
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING ifStatement {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING whileStatement {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING doStatement {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING subroutineCall {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING expressionList {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING returnStatement {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING expression {%s}\n", t.lx);
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
	if (SHOWDEBUG)
		printf("PARSED expression\n");
	// no error encountered
	return InfoNoError;
}
// relationalExpression→ arithmeticExpression {( = | > | < ) arithmeticExpression }
ParserInfo relationalExpression()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING relationalExpression {%s}\n", t.lx);
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
	if (SHOWDEBUG)
		printf("PARSED relationalExpression\n");
	// no error encountered
	return InfoNoError;
}
// arithmeticExpression → term {( + | - ) term }
ParserInfo arithmeticExpression()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING arithmeticExpression {%s}\n", t.lx);
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
	if (SHOWDEBUG)
		printf("PARSED arithmeticExpression\n");
	// no error encountered
	return InfoNoError;
}
// term → factor {( * | / ) factor }
ParserInfo term()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING term {%s}\n", t.lx);
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
	if (SHOWDEBUG)
		printf("PARSED term\n");
	// if we have reached here there is a term parsed and no error
	return InfoNoError;
}
// factor →( - | ~ |ϵ) operand
ParserInfo factor()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING factor {%s}\n", t.lx);
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
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING dotIdentifier {%s}\n", t.lx);
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
	// (
	Token next_token = GetNextToken();
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
// wrappedExpression → ( expression )
ParserInfo wrappedExpression()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING wrappedExpression {%s}\n", t.lx);
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
// operand → integerConstant | identifier [.identifier][[ expression ]|( expressionList ) ] | ( expression ) | stringLiteral | true | false | null | this
ParserInfo operand()
{
	Token t = PeekNextToken();
	if (SHOWDEBUG)
		printf("PARSING operand {%s}\n", t.lx);
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
		// ( expression )
		info = wrappedExpression();
		if (info.er != none)
			return info;
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
			// [expression]
			// eat the [
			GetNextToken();
			// expressionList
			ParserInfo info = expressionList();
			if (info.er != none)
				return info;
			// )
			next_token = GetNextToken();
			if (strcmp(next_token.lx, "]") != 0)
			{
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
	InitParser("./testfiles/Main.jack");
	ParserInfo info = Parse();
	printf("(%d,%s) near line %d\n", info.er, info.tk.lx, info.tk.ln);
	printf("End\n");
	return 1;
}
#endif
