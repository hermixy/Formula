// Copyright (C) 2015-2016, kylinsage <kylinsage@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

%{
 
#include "Function.h"
#include "parser.h"
#include "lexer.h"
 
// Request verbose, specific error message strings when yyerror is called.
#define YYERROR_VERBOSE 1

// Function yyerror is called whenever bison detects a syntax error
void yyerror (YYLTYPE *locp, Function *function, yyscan_t scanner, char const *msg) {
	std::cout << locp->first_line << "," << locp->first_column << ": syntax error!" << std::endl;
}
 
%}
 
%code requires {
 
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
 
}
 
%output  "parser.cpp"
%defines "parser.h"
 
%locations
%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { Function *function}
%parse-param { yyscan_t scanner }
 
%union {
	double real;	// real constant
	int integer;	// integer constant
	int index;		// register index
}
 
%token <real> TOKEN_REAL
%token <integer> TOKEN_INTEGER
 
%type <index> additive_expression
%type <index> multiplicative_expression
%type <index> unary_expression
%type <index> exponential_expression
%type <index> primary_expression
%type <index> constant
 
%%

calculation_unit
	: additive_expression 
	{
		// Ensure the function is finished
		function->addCode(Code(Code::Return, 0, 0, 0), @1.last_line);
	}
	;
	
additive_expression
	: multiplicative_expression
	| additive_expression[L] '+' multiplicative_expression[R]
	{
		int temp = function->newTemp($L, $R);
		function->addCode(Code(Code::Add, $L, $R, temp), @2.first_line);
		$$ = temp;
	}
	| additive_expression[L] '-' multiplicative_expression[R]
	{
		int temp = function->newTemp($L, $R);
		function->addCode(Code(Code::Sub, $L, $R, temp), @2.first_line);
		$$ = temp;
	}
	;

multiplicative_expression
	: unary_expression
	| multiplicative_expression[L] '*' unary_expression[R] 
	{
		int temp = function->newTemp($L, $R);
		function->addCode(Code(Code::Mul, $L, $R, temp), @2.first_line);
		$$ = temp;
	}
	| multiplicative_expression[L] '/' unary_expression[R]
	{
		int temp = function->newTemp($L, $R);
		function->addCode(Code(Code::Div, $L, $R, temp), @2.first_line);
		$$ = temp;
	}
	;
	
unary_expression
	: exponential_expression
	| '+' exponential_expression { $$ = $2; }
	| '-' exponential_expression
	{
		int temp = function->newTemp($2);
		function->addCode(Code(Code::Minus, $2, 0, temp), @2.first_line);
		$$ = temp;
	}
	;
	
exponential_expression
	: primary_expression
	| primary_expression[L] '^' exponential_expression[R] /* right associativity */
	{
		int temp = function->newTemp($L, $R);
		function->addCode(Code(Code::Pow, $L, $R, temp), @2.first_line);
		$$ = temp;
	}
	;
	
primary_expression
	: '(' additive_expression ')'  { $$ = $2; }
	| constant
	;

constant
	: TOKEN_INTEGER
	{
		// Index(negative) of constant in function's constants list
		$$ = -function->addConstant(Operand($1));
	}
	| TOKEN_REAL
	{
		$$ = -function->addConstant(Operand($1));
	}
	;
	
%%
