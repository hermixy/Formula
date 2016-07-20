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

#include <string>
#include "Function.h"
#include "parser.h"
#include "lexer.h"
#include "VM.h"

using std::string;

int yyparse(Function * function, void *scanner);

bool parse(Function *function, const char *expr)
{
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    if (yylex_init(&scanner)) {
        // couldn't initialize
        return false;
    }

    state = yy_scan_string(expr, scanner);

    if (yyparse(function, scanner)) {
        // error parsing
        return false;
    }

    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return true;
}

void showMessage() 
{
    std::cout << "Formula 2.0.0\nCalculating arithmetic expressions.\n";
}

int main(void)
{
    Function function("main");
    string expression;
    showMessage();
    std::cout << ">>";
    while(getline(std::cin, expression)){
        function.clearCodes();
        if(parse(&function, expression.c_str())) {
            std::cout << function << std::endl;
			VM vm(&function);
            vm.run();
        }
        std::cout << ">>";
    }

    return 0;
}
