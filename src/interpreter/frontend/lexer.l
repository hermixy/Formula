/*
	Copyright (C) 2015-2016, kylinsage <kylinsage@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

%{
#include "Function.h"
#include "parser.h"
#include <stdio.h>

/* Handling locations */
/* Locations */
int row;
int column;
/* Initialization */
#define YY_USER_INIT row = 1; column = 1;
/* Invoke for each token recognized by yylex, before calling the action code */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = row; \
        yylloc->first_column = column; yylloc->last_column = column + yyleng - 1; \
        column += yyleng;
%}
 
%option outfile="lexer.cpp" header-file="lexer.h"
%option warn nodefault
 
%option reentrant noyywrap never-interactive nounistd
%option bison-bridge
%option bison-locations
 
digits      [0-9]+
exponent 	([E|e][+|-]?[0-9]+)
WS          [ \r\n\t]*
 
%%

{digits}{exponent} | 
{digits}"."{exponent}? | 
"."{digits}{exponent}? |
{digits}"."{digits}{exponent}?	{				
    sscanf(yytext, "%lf", &yylval->real);
	return TOKEN_REAL; 
}
	
{digits}  			{
	sscanf(yytext, "%d", &yylval->integer);
	return TOKEN_INTEGER; 
}

[_a-zA-Z][_a-zA-Z0-9]*			{ 
	yylval->id.str = yytext;
	yylval->id.len = yyleng;
	return TOKEN_IDENTIFIER; 
}

{WS}	{ /* skip  */ }
"("		{ return '('; }
")"		{ return ')'; }
"+"		{ return '+'; }
"-"		{ return '-'; }
"*"		{ return '*'; }
"/"		{ return '/'; }
"^"		{ return '^'; }
"="		{ return '='; }
.		{ /*error*/
	return *yytext;
}

%%
