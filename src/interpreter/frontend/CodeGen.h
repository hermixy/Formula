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

#ifndef CODEGEN_H
#define CODEGEN_H

class Function;
class SemanticInfo;
class Semantic;

// Retrieve defined symbol, either local symbol or upvalue
bool retrieveSymbol(Function *function, SemanticInfo *info);
// Enter new symbol which is not defined
bool enterSymbol(Function *function, SemanticInfo *info);

// Move the last expression value of s to temperaries.
// All expression except the last expression of s are already temperaries.
// If the last expression of s is FunctionCall, set its expected results count to 1
// Note that function call is temperary value. Constants and locals will be moved to tempraries.
void makeSequence(Function *function, Semantic *exprs, int lineno);

// Translate as boolean expression
void codegenBoolean(Function *function, Semantic *exp, int lineno);

void codegenAsgnStmt(Function *function, SemanticInfo *target, int index, int lineno);

#endif /* CODEGEN_H */
