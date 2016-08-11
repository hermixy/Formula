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
	int codeid; // code index
}

/* The %destructor directive defines code that is called when a symbol is automatically discarded during error recovery. */
%destructor { 
	printf("%d,%d: free discarded symbols\n", @$.first_line, @$.first_column);
	destroy($$);
} <info>
 
%token OPERATOR_GE
%token OPERATOR_GT
%token OPERATOR_LE
%token OPERATOR_LT
%token OPERATOR_EQ
%token OPERATOR_NE
%token OPERATOR_AND
%token OPERATOR_NOT
%token OPERATOR_OR

%token TOKEN_RETURN
%token TOKEN_FUNCTION
%token TOKEN_FOR
%token TOKEN_WHILE
%token TOKEN_IF
%token TOKEN_ELSE
%token TOKEN_THEN
%token TOKEN_DO
%token TOKEN_END
%token <real> TOKEN_REAL
%token <integer> TOKEN_INTEGER
%token <id> TOKEN_IDENTIFIER
 
%type <info> if_condition 
%type <info> for_range 
%type <info> range_expression 
%type <info> target_list 
%type <info> target
%type <info> expression_list 
%type <info> optional_parameter_list 
%type <info> parameter_list 
%type <info> parameter 
%type <info> optional_argument_list 
%type <info> expression
%type <info> logical_or_expression
%type <info> logical_and_expression
%type <info> equality_expression
%type <info> relational_expression
%type <info> function_definition 
%type <info> additive_expression
%type <info> multiplicative_expression
%type <info> unary_expression
%type <info> exponential_expression
%type <info> postfix_expression
%type <info> function_call 
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
	| iteration_statement
	| selection_statement
	| return_statement
	| TOKEN_FUNCTION TOKEN_IDENTIFIER '(' optional_parameter_list ')'
	{
		int funcindex = function->createChild("anonymous");
		int temp = function->localSymbolCount();
		function->addLocalSymbolInfo(LocalSymbolInfo(string($2.str, $2.len), temp));
		function->addCode(Code(Code::Closure, funcindex, 0, temp), @1.first_line);
		function = function->getChild(funcindex);
		//$<info>$ = new Semantic(new SemanticInfo(SemanticInfo::FunctionDefinition, temp));
		if($4) { // Define all the parameters
			int m = count($4);
			auto param = $4;
			for(int i = 0; i < m; ++i) {
				function->addParam(LocalSymbolInfo(param->info->name, i));
				param = param->next;
			}
		}
		destroy($4);
	}
	statement_list TOKEN_END
	{
		function->addCode(Code(Code::Return, 0, 0, 0), @8.first_line);
		function = function->getParent();
		//$$ = $<info>6;
	}
	;

return_statement
	: TOKEN_RETURN expression_list
	{
		int n = count($2);
		// Move the last value to tempraries. Note that the first n-1 $3ession values and function call result(s) are  already temparies.
		if($2->prev->info->index < function->localSymbolCount()) {
			int temp = function->newTemp();
			function->addCode(Code(Code::Move, $2->prev->info->index, 0, temp), @1.first_line);
			$2->prev->info->index = temp;
		}

		if($2->prev->info->type == SemanticInfo::FunctionCall) {
			function->addCode(Code(Code::Return, $2->info->index, -1, 0), @1.first_line);
		} else {
			function->addCode(Code(Code::Return, $2->info->index, n, 0), @1.first_line);
		}
		destroy($2);
	}
	;

selection_statement
	: if_condition statement_list TOKEN_END
	{
		function->closeScope();
		function->backpatch($1->info->fc);
	}
	| if_condition statement_list TOKEN_ELSE 
	{
		function->closeScope();
		$1->info->tc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		function->backpatch($1->info->fc);
		function->openScope();
	}
	statement_list TOKEN_END
	{
		function->closeScope();
		function->backpatch($1->info->tc);
	}
	;

if_condition
	: TOKEN_IF expression TOKEN_THEN 
	{
		codegenBoolean(function, $2, @1.first_line);
		$$ = $2;
		std::cout << "cond: tc: " << $2->info->tc << " fc: " << $2->info->fc << std::endl;
		function->openScope();
		function->backpatch($2->info->tc);
	}
	;

iteration_statement
	: TOKEN_FOR TOKEN_IDENTIFIER '=' for_range 
	{
		function->addLocalSymbolInfo(string($2.str, $2.len));
		function->shrinkTemp();

		int start = function->addCode(Code(Code::ForPrep, $4->prev->info->index, 0, -1), @1.first_line);
		$<info>$ = new Semantic(new SemanticInfo(SemanticInfo::Expression, start));
	}
	TOKEN_DO statement_list TOKEN_END
	{
		int start = $<info>5->info->index;
		int end = function->addCode(Code(Code::ForLoop, $4->info->index, 0, start+1), @1.first_line);
		function->backpatch(start, end);
		function->closeScope();
		destroy($4);
	}
	| TOKEN_WHILE 
	{
		$<codeid>$ = function->codeSize();
	}
	expression 
	{
		codegenBoolean(function, $3, @1.first_line);
		function->openScope();
		function->backpatch($3->info->tc);
	}
	TOKEN_DO statement_list TOKEN_END
	{
		function->closeScope();
		function->addCode(Code(Code::Jmp, 0, 0, $<codeid>2), @6.last_line);
		function->backpatch($3->info->fc);
	}
	;

for_range
	: range_expression
	{
		int c = function->addConstant(Operand(1));
		int temp = function->newTemp();
		function->addCode(Code(Code::Move, -c, 0, temp), @1.first_line);

		function->addLocalSymbolInfo("(for_step)");
		function->shrinkTemp();
		$$ = $1;
	}
	| range_expression ',' expression
	{
		makeSequence(function, $3, @3.first_line);
		function->addLocalSymbolInfo("(for_step)");
		function->shrinkTemp();
		$$ = $1;
		destroy($3);
	}
	;

range_expression
	: expression ',' 
	{
		function->openScope();
		makeSequence(function, $1, @1.first_line);
		function->addLocalSymbolInfo("(for_start)");
		function->shrinkTemp();
	}
	expression 
	{
		makeSequence(function, $4, @4.first_line);
		function->addLocalSymbolInfo("(for_end)");
		function->shrinkTemp();
		$$ = $1;
		destroy($4);
	}
	;

assignment_statement
	: target_list '=' expression_list 
	{
		int m = count($1);
		int n = count($3);
		// Move the last value to tempraries. Note that the first n-1 $3ession values and function call result(s) are  already temparies.
		if((n > 1 && $3->prev->info->type != SemanticInfo::FunctionCall) || $3->prev->info->type == SemanticInfo::Boolean) {
			if($3->prev->info->type != SemanticInfo::Boolean) {
				int temp = function->newTemp();
				function->addCode(Code(Code::Move, $3->prev->info->index, 0, temp), @1.first_line);
				$3->prev->info->index = temp;
			} else {
				std::cout << "bool\n";
				int temp = function->newTemp();
				int tend = function->addCode(Code(Code::Bool, 1, 0, temp), @1.first_line);
				int jend = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
				int fend = function->addCode(Code(Code::Bool, 0, 0, temp), @1.first_line);
				function->backpatch($3->prev->info->tc, tend);
				function->backpatch($3->prev->info->fc, fend);
				function->backpatch(jend);
				$3->prev->info->index = temp;
			}
		}
		
		// When the last expression is function call, then backpatch it
		if($3->prev->info->type == SemanticInfo::FunctionCall) {
			int nresults; // results count expected
			if(m == n) nresults = 1;
			else if(m > n) nresults = m-n+1;
			else nresults = 0;
			function->backpatch($3->prev->info->codeIndex, nresults);
		} else if(m > n) {
		// Extend $3ession values with nils
			int temp = function->newTemp();
			function->addCode(Code(Code::Nil, temp, m-n, 0), @1.first_line);
		}
		
		// Retrieve symbols or enter symbols if not defined
		auto target = $1;
		for(int i = 0; i < m; ++i) {
			if(!retrieveSymbol(function, target->info))
				enterSymbol(function, target->info);
			target = target->next;
		}

		// initialize
		target = $1->prev;
		int cnt = m;
		int index = $3->info->index+m-1;
		// Assign the last target
		// if the last expression is temperary, rewrite code
		if($3->prev->info->index >= function->localSymbolCount()) {
			auto code = function->getCode(function->codeSize()-1);
			code->result = target->info->index;
			cnt--; 
			index--;
		}

		// Assign from back to front
		for(int i = 0; i < cnt; ++i) {
			codegenAsgnStmt(function, target->info, index, @1.first_line);
			index--;
			target = target->prev;
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
	: expression
	| expression_list ',' 
	{
		makeSequence(function, $1, @1.first_line);
	}
	expression 
	{
		concat($1, $4);
		$$ = $1;
	}
	;

expression
	: logical_or_expression
	| function_definition
	;

function_definition
	: TOKEN_FUNCTION '(' optional_parameter_list ')'
	{
		int funcindex = function->createChild("anonymous");
		int temp = function->newTemp();
		function->addCode(Code(Code::Closure, funcindex, 0, temp), @1.first_line);
		function = function->getChild(funcindex);
		$<info>$ = new Semantic(new SemanticInfo(SemanticInfo::FunctionDefinition, temp));
		if($3) { // Define all the parameters
			int m = count($3);
			auto param = $3;
			for(int i = 0; i < m; ++i) {
				function->addParam(LocalSymbolInfo(param->info->name, i));
				param = param->next;
			}
		}
		destroy($3);
	}
	statement_list TOKEN_END
	{
		function->addCode(Code(Code::Return, 0, 0, 0), @7.first_line);
		function = function->getParent();
		$$ = $<info>5;
	}
	;

optional_parameter_list
	:/* empty */ { $$ = nullptr; }
	| parameter_list
	;

parameter_list
	: parameter
	| parameter_list ',' parameter
	{
		concat($1, $3);
		$$ = $1;
	}
	;

parameter
	: TOKEN_IDENTIFIER
	{
		$$ = new Semantic(new SemanticInfo(SemanticInfo::Identifier, string($1.str, $1.len)));
	}
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression[L] OPERATOR_OR 
	{
		codegenBoolean(function, $L, @1.first_line);
		function->backpatch($L->info->fc);
	}
	logical_and_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->fc = $R->info->fc;
		$$->info->tc = function->merge($L->info->tc, $R->info->tc);
		destroy($L);
		destroy($R);
	}
	;

logical_and_expression
	: equality_expression
	| logical_and_expression[L] OPERATOR_AND 
	{
		codegenBoolean(function, $L, @1.first_line);
		function->backpatch($L->info->tc);
	}
	equality_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = $R->info->tc;
		$$->info->fc = function->merge($L->info->fc, $R->info->fc);
		destroy($L);
		destroy($R);
	}
	;

equality_expression
	: relational_expression
	| equality_expression[L] OPERATOR_EQ relational_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = function->addCode(Code(Code::Jeq, $L->info->index, $R->info->index, -1), @1.first_line);
		$$->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		destroy($L);
		destroy($R);
	}
	| equality_expression[L] OPERATOR_NE relational_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = function->addCode(Code(Code::Jne, $L->info->index, $R->info->index, -1), @1.first_line);
		$$->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		destroy($L);
		destroy($R);
	}
	;

relational_expression
	: additive_expression
	| relational_expression[L] OPERATOR_GE additive_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = function->addCode(Code(Code::Jge, $L->info->index, $R->info->index, -1), @1.first_line);
		$$->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		destroy($L);
		destroy($R);
	}
	| relational_expression[L] OPERATOR_GT additive_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = function->addCode(Code(Code::Jgt, $L->info->index, $R->info->index, -1), @1.first_line);
		$$->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		destroy($L);
		destroy($R);
	}
	| relational_expression[L] OPERATOR_LE additive_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = function->addCode(Code(Code::Jle, $L->info->index, $R->info->index, -1), @1.first_line);
		$$->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		destroy($L);
		destroy($R);
	}
	| relational_expression[L] OPERATOR_LT additive_expression[R]
	{
		$$ = new Semantic(new SemanticInfo(-1, -1));
		$$->info->tc = function->addCode(Code(Code::Jlt, $L->info->index, $R->info->index, -1), @1.first_line);
		$$->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), @1.first_line);
		destroy($L);
		destroy($R);
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
	| OPERATOR_NOT exponential_expression
	{
		codegenBoolean(function, $2, @1.first_line);
		int temp = $2->info->tc;
		$2->info->tc = $2->info->fc;
		$2->info->fc = temp;
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
	| function_call
	;

function_call
	: postfix_expression 
	{
		// Move the last expression value of $1 to temperaries
		// If the last expression of $1 is FunctionCall, set its expected results count to 1
		// Note that function call is temperary value. Constants and locals will be moved to tempraries.
		makeSequence(function, $1, @1.first_line);
		$<info>$ = new Semantic(new SemanticInfo(SemanticInfo::FunctionCall, $1->info->index, -1)); // to be filled later
	}
	'(' optional_argument_list ')'
	{
		int closureIndex = $<info>2->info->index;
		int codeIndex;
		// The 4th part of CALL instruction will be backpatched when assignment
		// Case 1: no arguments
		if(!$4) {
			codeIndex = function->addCode(Code(Code::Call, closureIndex, 0, -1), @1.first_line);
		// Case 2: the last argument is function call, the arguments count cann't be determinated
		} else if($4->prev->info->type == SemanticInfo::FunctionCall) {
			codeIndex = function->addCode(Code(Code::Call, closureIndex, -1, -1), @1.first_line);
		// Case 3: the others
		} else {
			codeIndex = function->addCode(Code(Code::Call, closureIndex, count($4), -1), @1.first_line);
		}
		destroy($1);
		destroy($4);
		$<info>2->info->codeIndex = codeIndex;
		$$ = $<info>2;
	}
	;

optional_argument_list
	: /* empty */ { $$ = nullptr; }
	| expression_list
	{
		// Move all expressions to tempraries
		if($1->prev->info->type != SemanticInfo::FunctionCall && $1->prev->info->index < function->localSymbolCount()) {
			int temp = function->newTemp();
			function->addCode(Code(Code::Move, $1->prev->info->index, 0, temp), @1.first_line);
			$1->prev->info->index = temp;
		}
		$$ = $1;
	}
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
			std::cout << "Undefined symbol:" << string($1.str, $1.len) << std::endl;
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
