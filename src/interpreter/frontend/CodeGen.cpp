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

#include "CodeGen.h"
#include "Function.h"
#include "Semantic.h"
#include <string>
using std::string;

bool retrieveSymbol(Function *function, SemanticInfo *info)
{
    int index;
    if((index = function->getLocalSymbol(info->name)) != -1) {
        info->index = index;
        info->type = SemanticInfo::LocalSymbol;
    } else if((index = function->getUpvalue(info->name)) != -1) {
        info->index = index;
        info->type = SemanticInfo::Upvalue;
    } else if((index = function->getParentLocalSymbol(info->name)) != -1) {
        index = function->addUpvalueInfo(UpvalueInfo(info->name, true, index));
        info->index = index;
        info->type = SemanticInfo::Upvalue;
    } else if((index = function->getParentLocalSymbol(info->name)) != -1) {
        index = function->addUpvalueInfo(UpvalueInfo(info->name, false, index));
        info->index = index;
        info->type = SemanticInfo::Upvalue;
    } else {
        return false;
    }
    return true;
}

bool enterSymbol(Function *function, SemanticInfo *info)
{
    info->index = function->addLocalSymbolInfo(LocalSymbolInfo(info->name, function->localSymbolCount()));
    info->type = SemanticInfo::LocalSymbol;

    return true;
}

void codegenAsgnStmt(Function *function, SemanticInfo *target, int index, int lineno)
{
    if(target->type == SemanticInfo::Upvalue) {
        function->addCode(Code(Code::SetUpval, index, 0, target->index), lineno);
    } else if(target->type == SemanticInfo::LocalSymbol) {
        if(target->index != index)
            function->addCode(Code(Code::Move, index, 0, target->index), lineno);
    } else {
        throw "Invalid assignment statement";
    }
}
