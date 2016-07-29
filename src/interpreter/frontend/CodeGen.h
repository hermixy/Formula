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

// Retrieve defined symbol, either local symbol or upvalue
bool retrieveSymbol(Function *function, SemanticInfo *info);
// Enter new symbol which is not defined
bool enterSymbol(Function *function, SemanticInfo *info);

void codegenAsgnStmt(Function *function, SemanticInfo *target, SemanticInfo *expression, int lineno);

#endif /* CODEGEN_H */
