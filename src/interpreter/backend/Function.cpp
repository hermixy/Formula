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
// along with this program.  If nOperand::, see <http://www.gnu.org/licenses/>.

#include "Function.h"
#include <iostream>

Code *Function::getBaseCode()
{
    return codes.empty()? nullptr : &codes[0];
}

std::size_t Function::codeSize() const
{
    return codes.size();
}

void Function::adjustSlotCount(const Code &code) 
{
    int n;
    switch(code.op) {
    case Code::Jnz:
    case Code::Jmp:
    case Code::Jgt:
    case Code::Jge:
    case Code::Jlt:
    case Code::Jle:
    case Code::Jeq:
    case Code::Jne:
    case Code::Return:
    case Code::SetUpval:
        break;
    case Code::Call:
        n = code.arg2 > code.result ? code.arg2 : code.result;
        n = n > 0 ? n : 0;
        nslots = code.arg1 + n > nslots ? code.arg1 + n : nslots;
    case Code::Move:
    case Code::Add:
    case Code::Sub:
    case Code::Mul:
    case Code::Div:
    case Code::Pow:
    case Code::Mod:
    case Code::Minus:
    case Code::GetUpval:
    case Code::Closure:
        nslots = code.result + 1 > nslots ? code.result + 1 : nslots;
        break;
    case Code::Nil:
        nslots = code.arg1 + code.arg2 - 1 > nslots ? code.arg1 + code.arg2 - 1 : nslots;
        break;
    default: break;
    }
}

Code *Function::getCode(std::size_t index)
{
    return &codes[index];
}

std::size_t Function::addCode(const Code &code, int line)
{
    codes.push_back(code);
    lines.push_back(line);
    adjustSlotCount(code);
    return codes.size() - 1;
}

std::size_t Function::addConstant(const Operand & a)
{
    for(auto i = 0; i < constants.size(); ++i)
        if( a == constants[i])
            return i;
    constants.push_back(a);
    return constants.size() - 1;
}

const Operand & Function::getConstant(int i) const
{
    return constants[i];

}

std::size_t Function::addLocalSymbolInfo(const LocalSymbolInfo &localInfo) 
{
    scopes.back().locals.push_back(localInfo);
    ntemps++;
    return localSymbolCount() - 1;
}

std::size_t Function::addLocalSymbolInfo(string name)
{
    scopes.back().locals.push_back(LocalSymbolInfo(name, localSymbolCount()));
    ntemps++;
    return localSymbolCount() - 1;
}

std::size_t Function::addParam(const LocalSymbolInfo &paramInfo) 
{
    scopes.back().locals.push_back(paramInfo);
    nparams++;
    ntemps++;
    return localSymbolCount() - 1;
}

std::size_t Function::addUpvalueInfo(const UpvalueInfo &upvalueInfo)
{
    upvalueInfos.push_back(upvalueInfo);
    return upvalueInfos.size() - 1;
}

std::size_t Function::createChild(string name)
{
    auto child = new Function(name);
    child->parent = this;
    children.push_back(child);
    return children.size() - 1;
}

Function *Function::getChild(std::size_t i)
{
    return children[i];
}

ostream & operator <<(ostream & os, const Function & f)
{
    // Function informations and instructions
    os << "\n" << f.name << " (" << f.codes.size() << " instructions at " << &f << ")" << std::endl;
    os << f.nparams << " params, "
       << f.nslots << " slots, "
       << f.upvalueInfos.size() << " upvalues, "
       << f.scopes[0].locals.size() << " locals, "
       << f.constantCount() << " constants, "
       << f.children.size() << " functions"
       << std::endl;

    // Codes
    for (auto i = 0; i < f.codes.size(); ++i)
        os << "\t" << i << "\t[" << f.lines[i] << "]\t" << opdesc[f.codes[i].op] << "\t" << f.codes[i].arg1
           << "\t" << f.codes[i].arg2 << "\t " << f.codes[i].result << std::endl;

    // Constants
    os << "constants (" << f.constantCount() << ") for " << &f << ":" << std::endl;
    for (auto i = 1; i < f.constants.size(); ++i)
        os << "\t" << i << "\t" << f.constants[i] << std::endl;

    // Locals
    os << "locals (" << f.scopes[0].locals.size() << ") for " << &f << ":" << std::endl;
    for (auto i = 0; i < f.scopes[0].locals.size(); ++i)
        os << "\t" << i << "\t" << f.scopes[0].locals[i].name << std::endl;


    // Upvalues
    os << "upvalues (" << f.upvalueInfos.size() << ") for " << &f << ":" << std::endl;
    for (auto i = 0; i < f.upvalueInfos.size(); ++i) {
        os << "\t" << i << "\t" << f.upvalueInfos[i].name
           << "\t" << f.upvalueInfos[i].isParentLocal
           << "\t" << f.upvalueInfos[i].registerIndex
           << std::endl;
    }

    for(auto child: f.children)
        os << *child;

    return os;
}

// Concatenate the lists pointed to by codelist1 and codelist2 
// and returns a pointer to the concatenated list
int Function::merge(int codelist1, int codelist2)
{
    if(codelist2 == -1)
        return codelist1;
    else {
        auto i = codelist2;
        while(codes[i].result != -1)
            i = codes[i].result;
        codes[i].result = codelist1;
        return codelist2;
    }
}

// Insert result as the target label for each of the instructions
// on the list pointed to by codelist
void Function::backpatch(int codelist, int result)
{
    auto i = codelist;
    while(codes[i].result != -1) {
        auto temp = codes[i].result;
        codes[i].result = result;
        i = temp;
    }
    codes[i].result = result;
}

void Function::backpatch(int codelist)
{
    backpatch(codelist, codeSize());
}

// Allocate temporary register index for binary expression result
int Function::newTemp(int index1, int index2)
{
    // Attention, locals.size() is an unsigned integer
    int count = localSymbolCount();
    if(index1 >= count && index2 >= count)
        return index1 < index2 ? index1 : index2;
    else if(index1 >= count)
        return index1;
    else if(index2 >= count)
        return index2;
    else
        return ntemps++;
}

// Allocate temporary register index for unary expression result
int Function::newTemp(int index)
{
    // Attention, locals.size() is an unsigned integer
    int count = localSymbolCount();
    if(index >= count)
        return index;
    else
        return ntemps++;
}

// Allocate temporary register index
int Function::newTemp()
{
    return ntemps++;
}

int Function::findUpvalue(string name) const
{
    int i = 0;
    for(auto &up : upvalueInfos) {
        if(up.name == name)
            return i;
        i++;
    }
    return -1;
}

// Reverse search
int Function::getLocalSymbol(string name) const
{
    int index = localSymbolCount();
    int m = scopes.size();
    for(int i = m-1; i >= 0; --i) {
        for(int j = scopes[i].locals.size()-1; j >= 0; --j) {
            --index;
            if(scopes[i].locals[j].name == name)
                return index;
        }
    }
    return -1;
}

int Function::getParentUpvalue(string name) const
{
    if(parent)
        return parent->findUpvalue(name);
    return -1;
}

int Function::getParentLocalSymbol(string name) const
{
    if(parent)
        return parent->getLocalSymbol(name);
    return -1;
}

void Function::reverseCodes(int start, int end)
{
    for(int i = start, j = end; i < j; ++i, --j) {
        auto temp = codes[i];
        codes[i] = codes[j];
        codes[j] = temp;
    }
}
