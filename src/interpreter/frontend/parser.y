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
#include "CodeGen.h"
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
	struct Semantic *info;
}

/* The %destructor directive defines code that is called when a symbol is automatically discarded during error recovery. */
%destructor { 
	printf("%d,%d: free discarded symbols\n", @$.first_line, @$.first_column);
	destroy($$);
} <info>
 
%token TOKEN_RETURN
%token <real> TOKEN_REAL
%token <integer> TOKEN_INTEGER
%token <id> TOKEN_IDENTIFIER
 
%type <info> target_list 
%type <info> target
%type <info> expression_list 
%type <info> additive_expression
%type <info> multiplicative_expression
%type <info> unary_expression
%type <info> exponential_expression
%type <info> postfix_expression
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
	: statement 
	| statement_list statement
	;

statement
	: assignment_statement
	| return_statement
	;

return_statement
	: TOKEN_RETURN
	{
		function->addCode(Code(Code::Return, 0, 0, 0), @1.last_line);
	}
	| TOKEN_RETURN expression_list
	{
		function->addCode(Code(Code::Return, $2->info->index, -1, 0), @1.last_line);
		destroy($2);
	}
	;

assignment_statement
	: target_list '=' expression_list 
	{
		int m = count($1);
		int n = count($3);
		auto target = $1;
		auto expr = $3;
		if(m == n) {
			// Move all values to tempraries
			for(int i = 0; i < n-1; ++i) {
				int temp = function->newTemp();
				function->addCode(Code(Code::Move, expr->info->index, 0, temp), @1.first_line);
				expr->info->index = temp;
				expr = expr->next;
			}
			// Retrieve symbols or enter symbols if not defined
			for(int i = 0; i < m; ++i) {
				if(!retrieveSymbol(function, target->info))
					enterSymbol(function, target->info);
				target = target->next;
			}

			target = target->prev; // point to the last target, expr points to the last expression
			// Assign from back to front
			for(int i = 0; i < m; ++i) {
				codegenAsgnStmt(function, target->info, expr->info, @1.first_line);
				target = target->prev;
				expr = expr->prev;
			}
		} else if(m < n) {
			// Move all values to tempraries
			for(int i = 0; i < n; ++i) {
				int temp = function->newTemp();
				function->addCode(Code(Code::Move, expr->info->index, 0, temp), @1.first_line);
				expr->info->index = temp;
				expr = expr->next;
			}
			// Retrieve symbols or enter symbols if not defined
			for(int i = 0; i < m; ++i) {
				if(!retrieveSymbol(function, target->info))
					enterSymbol(function, target->info);
				target = target->next;
			}

			target = target->prev; // point to the last target
			expr = expr->prev; // point to the last expression
			// Shift expr to proper position
			for(int i = n-1; i > m-1; --i)
				expr = expr->prev;
			// Assign from back to front
			for(int i = 0; i < m; ++i) {
				codegenAsgnStmt(function, target->info, expr->info, @1.first_line);
				target = target->prev;
				expr = expr->prev;
			}
		} else { // m > n
			// Move all values to tempraries
			for(int i = 0; i < n; ++i) {
				int temp = function->newTemp();
				function->addCode(Code(Code::Move, expr->info->index, 0, temp), @1.first_line);
				expr->info->index = temp;
				expr = expr->next;
			}
			// Assign the last expr to last m-n targets 
			int temp = function->newTemp();
			function->addCode(Code(Code::Nil, temp, m-n, 0), @1.first_line);
			int start = temp+m-n-1;
			//function->setTemp(function->tempCount()+m-n);
			// Retrieve symbols or enter symbols if not defined
			for(int i = 0; i < m; ++i) {
				if(!retrieveSymbol(function, target->info))
					enterSymbol(function, target->info);
				target = target->next;
			}

			target = target->prev; // point to the last target
			// Assign from back to front
			for(int i = 0; i < m; ++i) {
				if(start != target->info->index)
					function->addCode(Code(Code::Move, start, 0, target->info->index), @1.first_line);
				start--;
				target = target->prev;
			}
		}

		function->shrinkTemp();
		destroy($1);
		destroy($3);
	}
	;

target_list
	: target
	| target_list ',' target
	{
		concat($1, $3);
		$$ = $1;
	}
	;

target
	: TOKEN_IDENTIFIER
	{
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Identifier, string($1.str, $1.len)));
	}
	;

expression_list
	: additive_expression
	| expression_list ',' additive_expression
	{
		concat($1, $3);
		$$ = $1;
	}
	;
	
additive_expression
	: multiplicative_expression
	| additive_expression[L] '+' multiplicative_expression[R]
	{
		int temp = function->newTemp($L->info->index, $R->info->index);
		function->addCode(Code(Code::Add, $L->info->index, $R->info->index, temp), @2.first_line);
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Expression, temp));
		destroy($L);
		destroy($R);
	}
	| additive_expression[L] '-' multiplicative_expression[R]
	{
		int temp = function->newTemp($L->info->index, $R->info->index);
		function->addCode(Code(Code::Sub, $L->info->index, $R->info->index, temp), @2.first_line);
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Expression, temp));
		destroy($L);
		destroy($R);
	}
	;

multiplicative_expression
	: unary_expression
	| multiplicative_expression[L] '*' unary_expression[R] 
	{
		int temp = function->newTemp($L->info->index, $R->info->index);
		function->addCode(Code(Code::Mul, $L->info->index, $R->info->index, temp), @2.first_line);
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Expression, temp));
		destroy($L);
		destroy($R);
	}
	| multiplicative_expression[L] '/' unary_expression[R]
	{
		int temp = function->newTemp($L->info->index, $R->info->index);
		function->addCode(Code(Code::Div, $L->info->index, $R->info->index, temp), @2.first_line);
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Expression, temp));
		destroy($L);
		destroy($R);
	}
	;
	
unary_expression
	: exponential_expression
	| '+' exponential_expression { $$ = $2; }
	| '-' exponential_expression
	{
		int temp = function->newTemp($2->info->index);
		function->addCode(Code(Code::Minus, $2->info->index, 0, temp), @2.first_line);
		$2->info->index = temp;
		$$ = $2;
	}
	;
	
exponential_expression
	: postfix_expression
	| postfix_expression[L] '^' exponential_expression[R] /* right associativity */
	{
		int temp = function->newTemp($L->info->index, $R->info->index);
		function->addCode(Code(Code::Pow, $L->info->index, $R->info->index, temp), @2.first_line);
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Expression, temp));
		destroy($L);
		destroy($R);
	}
	;

postfix_expression
	: primary_expression
	| postfix_expression '(' ')'
	{
		int temp = function->newTemp();
		function->addCode(Code(Code::Move, $1->info->index, 0, temp), @1.first_line);
		int codeIndex = function->addCode(Code(Code::Call, temp, 0, -1), @1.first_line);
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Call, codeIndex));
	}
	/*| postfix_expression '(' expression_list ')'
	{
		int closureIndex = function->newTemp();
		function->addCode(Code(Code::Move, $1->info->index, 0, temp), @1.first_line);
		auto expression = $3;
		int count = 0;
		while(expression) {
			int temp = function->newTemp();
			function->addCode(Code(Code::Move, expression->info->index, 0, temp), @1.first_line);
			count++;
			expression =  expression->next;
		}
		function->addCode(Code(Code::Call, closureIndex, count, 0), @1.first_line);
	}*/
	;
	
primary_expression
	: '(' additive_expression ')'  { $$ = $2; }
	| constant
	| TOKEN_IDENTIFIER 
	{
		int index;
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Identifier, string($1.str, $1.len)));
		if(retrieveSymbol(function, $$->info)) {
			if($$->info->type == SemanticInfo::LocalSymbol) {
				// do nothing
			} else if($$->info->type == SemanticInfo::Upvalue) {
				int temp = function->newTemp();
				function->addCode(Code(Code::GetUpval, $$->info->index, 0, temp), @1.first_line);
				$$->info->index = temp;
			}
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
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Constant, -index));
	}
	| TOKEN_REAL
	{
		auto index = function->addConstant(Operand($1));
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Constant, -index));
	}
	;
	
%%
