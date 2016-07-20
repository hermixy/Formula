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
#include "Semantic.h"
#include <string>
 
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
	struct {
		const char *str;
		int len;
	}id;	// identifier 
	struct SemanticInfo *info;
}

/* The %destructor directive defines code that is called when a symbol is automatically discarded during error recovery. */
%destructor { 
	printf("%d,%d: free discarded symbols\n", @$.first_line, @$.first_column);
	delete $$;
}<info>
 
%token <real> TOKEN_REAL
%token <integer> TOKEN_INTEGER
%token <id> TOKEN_IDENTIFIER
 
%type <info> additive_expression
%type <info> multiplicative_expression
%type <info> unary_expression
%type <info> exponential_expression
%type <info> primary_expression
%type <info> constant
 
%%

translation_unit
	: statement_list
	{
		// Ensure the function is finished
		function->addCode(Code(Code::Return, 0, 0, 0), @1.last_line);
	}
	;

statement_list
	: assignment_statement 
	| statement_list assignment_statement
	;

assignment_statement
	: TOKEN_IDENTIFIER '=' additive_expression
	{
		int index;
		string name = string($1.str, $1.len);
		if((index = function->getLocalSymbol(name)) != -1) {
			if($3->index != index)
				function->addCode(Code(Code::Move, $3->index, 0, index), @1.first_line);
		} else if((index = function->getUpvalue(name)) != -1) {
			function->addCode(Code(Code::SetUpval, $3->index, 0, index), @1.first_line);
		} else if((index = function->getParentLocalSymbol(name)) != -1) {
			index = function->addUpvalueInfo(UpvalueInfo(name, true, index));
			function->addCode(Code(Code::SetUpval, $3->index, 0, index), @1.first_line);
		} else if((index = function->getParentLocalSymbol(name)) != -1) {
			index = function->addUpvalueInfo(UpvalueInfo(name, false, index));
			function->addCode(Code(Code::SetUpval, $3->index, 0, index), @1.first_line);
		} else {
			index = function->addLocalSymbolInfo(LocalSymbolInfo(name, function->localSymbolCount()));
			if($3->index != index)
				function->addCode(Code(Code::Move, $3->index, 0, index), @1.first_line);
		}
		delete $3;
	}
	;
	
additive_expression
	: multiplicative_expression
	| additive_expression[L] '+' multiplicative_expression[R]
	{
		int temp = function->newTemp($L->index, $R->index);
		function->addCode(Code(Code::Add, $L->index, $R->index, temp), @2.first_line);
		$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		delete $L;
		delete $R;
	}
	| additive_expression[L] '-' multiplicative_expression[R]
	{
		int temp = function->newTemp($L->index, $R->index);
		function->addCode(Code(Code::Sub, $L->index, $R->index, temp), @2.first_line);
		$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		delete $L;
		delete $R;
	}
	;

multiplicative_expression
	: unary_expression
	| multiplicative_expression[L] '*' unary_expression[R] 
	{
		int temp = function->newTemp($L->index, $R->index);
		function->addCode(Code(Code::Mul, $L->index, $R->index, temp), @2.first_line);
		$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		delete $L;
		delete $R;
	}
	| multiplicative_expression[L] '/' unary_expression[R]
	{
		int temp = function->newTemp($L->index, $R->index);
		function->addCode(Code(Code::Div, $L->index, $R->index, temp), @2.first_line);
		$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		delete $L;
		delete $R;
	}
	;
	
unary_expression
	: exponential_expression
	| '+' exponential_expression { $$ = $2; }
	| '-' exponential_expression
	{
		int temp = function->newTemp($2->index);
		function->addCode(Code(Code::Minus, $2->index, 0, temp), @2.first_line);
		$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		delete $2;
	}
	;
	
exponential_expression
	: primary_expression
	| primary_expression[L] '^' exponential_expression[R] /* right associativity */
	{
		int temp = function->newTemp($L->index, $R->index);
		function->addCode(Code(Code::Pow, $L->index, $R->index, temp), @2.first_line);
		$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		delete $L;
		delete $R;
	}
	;
	
primary_expression
	: '(' additive_expression ')'  { $$ = $2; }
	| constant
	| TOKEN_IDENTIFIER 
	{
		int index;
		string name = string($1.str, $1.len);
		if((index = function->getLocalSymbol(name)) != -1) {
			$$ = new SemanticInfo(SemanticInfo::Expression, index, nullptr);
		} else if((index = function->getUpvalue(name)) != -1) {
			int temp = function->newTemp();
			function->addCode(Code(Code::GetUpval, index, 0, temp), @1.first_line);
			$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		} else if((index = function->getParentLocalSymbol(name)) != -1) {
			index = function->addUpvalueInfo(UpvalueInfo(name, true, index));
			int temp = function->newTemp();
			function->addCode(Code(Code::GetUpval, index, 0, temp), @1.first_line);
			$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		} else if((index = function->getParentUpvalue(name)) != -1) {
			index = function->addUpvalueInfo(UpvalueInfo(name, false, index));
			int temp = function->newTemp();
			function->addCode(Code(Code::GetUpval, index, 0, temp), @1.first_line);
			$$ = new SemanticInfo(SemanticInfo::Expression, temp, nullptr);
		} else {
			std::cout << "Undefined symbol" << std::endl;
			YYERROR;
		}
	}
	;

constant
	: TOKEN_INTEGER
	{
		// Index(negative) of constant in function's constants list
		auto index = function->addConstant(Operand($1));
		$$ = new SemanticInfo(SemanticInfo::Constant, -index, nullptr);
	}
	| TOKEN_REAL
	{
		auto index = function->addConstant(Operand($1));
		$$ = new SemanticInfo(SemanticInfo::Constant, -index, nullptr);
	}
	;
	
%%
