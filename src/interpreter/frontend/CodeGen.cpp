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
#include <vector>
using std::string;
using std::vector;

bool retrieveSymbol(Function *function, SemanticInfo *info)
{
    int index;
    if((index = function->getLocalSymbol(info->name)) != -1) {
        info->index = index;
        info->type = SemanticInfo::LocalSymbol;
        return true;
    } else if((index = function->findUpvalue(info->name)) != -1) {
        info->index = index;
        info->type = SemanticInfo::Upvalue;
        return true;
    }

    vector<Function *> funcs;
    funcs.push_back(function);
    auto p = function->getParent();
    while(p) {
        if((index = p->getLocalSymbol(info->name)) != -1) {
            int i = funcs.size()-1;
            index = funcs[i]->addUpvalueInfo(UpvalueInfo(info->name, true, index));
            --i;
            for(; i >= 0; --i)
                index = funcs[i]->addUpvalueInfo(UpvalueInfo(info->name, false, index));

            info->index = index;
            info->type = SemanticInfo::Upvalue;
            return true;
        } else if((index = p->findUpvalue(info->name)) != -1) {
            for(int i = funcs.size()-1; i >= 0; --i)
                index = funcs[i]->addUpvalueInfo(UpvalueInfo(info->name, false, index));

            info->index = index;
            info->type = SemanticInfo::Upvalue;
            return true;
        } else {
            funcs.push_back(p);
            p = p->getParent();
        }
    }

    return false;
}

bool enterSymbol(Function *function, SemanticInfo *info)
{
    info->index = function->addLocalSymbolInfo(LocalSymbolInfo(info->name, function->localSymbolCount()));
    info->type = SemanticInfo::LocalSymbol;

    return true;
}

void makeSequence(Function *function, Semantic *exprs, int lineno)
{
    if(exprs->prev->info->type == SemanticInfo::FunctionCall) {
        function->backpatch(exprs->prev->info->codeIndex, 1);
        function->setTemp(exprs->prev->info->index+1);
    } else if(exprs->prev->info->index < function->localSymbolCount()) {
        if(exprs->prev->info->type != SemanticInfo::Boolean) {
            int temp = function->newTemp();
            function->addCode(Code(Code::Move, exprs->prev->info->index, 0, temp), lineno);
            exprs->prev->info->index = temp;
        } else {
            int temp = function->newTemp();
            int tend = function->addCode(Code(Code::Bool, 1, 0, temp), lineno);
            int jend = function->addCode(Code(Code::Jmp, 0, 0, -1), lineno);
            int fend = function->addCode(Code(Code::Bool, 0, 0, temp), lineno);
            function->backpatch(exprs->prev->info->tc, tend);
            function->backpatch(exprs->prev->info->fc, fend);
            function->backpatch(jend);
            exprs->prev->info->index = temp;
        }
    }
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

void codegenBoolean(Function *function, Semantic *exp, int lineno)
{
    if(exp->info->type != SemanticInfo::Boolean) {
        exp->info->type = SemanticInfo::Boolean;
        exp->info->tc = function->addCode(Code(Code::Jnz, exp->info->index, 0, -1), lineno);
        exp->info->fc = function->addCode(Code(Code::Jmp, 0, 0, -1), lineno);
    }
}
