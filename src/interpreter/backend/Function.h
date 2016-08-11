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

#ifndef FUNCTION_H
#define FUNCTION_H

#include "Code.h"
#include "Operand.h"
#include <vector>
#include <iostream>
#include <string>
using std::string;

// Information of upvalues
struct UpvalueInfo {
    // Upvalue name
    string name;

    // This upvalue is parent function's local variable
    // when value is true, otherwise it is parent parent
    // (... and so on) function's local variable
    bool isParentLocal;

    // Register id when this upvalue is parent function's
    // local variable, otherwise it is index of upvalue list
    // of parent function
    int registerIndex;

    UpvalueInfo(string name, bool isParentLocal, int registerIndex)
        :name(name), isParentLocal(isParentLocal), registerIndex(registerIndex) {
    }
};

// Information of local symbols, including variable and function
struct LocalSymbolInfo {
    // Symbol name
    string name;

    // Register id in function
    int registerIndex;

    LocalSymbolInfo(string name, int registerIndex)
        :name(name), registerIndex(registerIndex) {
    }
};

// Symbol scope
struct SymbolScope {
    std::vector<LocalSymbolInfo> locals;
};

// Function prototype class, all runtime functions(closures) reference this
// class object. This class contains some static information generated after parsing.
class Function {
public:
    Function(string name):name(name), nparams(0), nresults(0), nslots(0), ntemps(0), parent(nullptr) {
        constants.push_back(Operand());
        scopes.push_back(SymbolScope());
    }

    Function(const Function &) = delete;
    Function & operator = (const Function &) = delete;

    ~Function() {
        for(auto child: children)
            delete child;
    }

    // Function instructions and size
    Code *getBaseCode();
    void clearCodes() {
        codes.clear();
        ntemps = localSymbolCount();
    }
    std::size_t codeSize()const;
    Code *getCode(std::size_t i);
    std::size_t addCode(const Code &code, int line);
    void reverseCodes(int start, int end);

    std::size_t addConstant(const Operand & c);
    const Operand & getConstant(int i) const;

    std::size_t addLocalSymbolInfo(const LocalSymbolInfo &localInfo);
    std::size_t addLocalSymbolInfo(string name);

    std::size_t addParam(const LocalSymbolInfo &paramInfo);

    void adjustSlotCount(const Code &code);
    int slotCount() const {
        return nslots;
    }

    int paramCount() const {
        return nparams;
    }

    int resultCount() const {
        return nresults;
    }

    int constantCount() const {
        return constants.size() - 1;
    }

    int localSymbolCount() const {
        int count = 0;
        for(int i = scopes.size()-1; i >=0; --i)
            count += scopes[i].locals.size();
        return count;
    }

    std::size_t addUpvalueInfo(const UpvalueInfo &upvalueInfo);

    int upvalueCount() const {
        return upvalueInfos.size();
    }

    const UpvalueInfo *getUpvalueInfo(std::size_t index) const {
        return &upvalueInfos[index];
    }

    int findUpvalue(string name) const;
    int getLocalSymbol(string name) const;
    int getParentUpvalue(string name) const;
    int getParentLocalSymbol(string name) const;

    std::size_t createChild(string name);
    Function * getChild(std::size_t index);
    Function * getParent() {
        return parent;
    }

    friend ostream & operator <<(ostream & os, const Function & f);

    // Concatenate the lists pointed to by codelist1 and codelist2
    // and returns a pointer to the concatenated list
    int merge(int codelist1, int codelist2);
    // Insert i as the target label for each of the instructions
    // on the list pointed to by codelist
    void backpatch(int codelist, int i);
    // Insert current code size as the target label for each of the instructions
    // on the list pointed to by codelist
    void backpatch(int codelist);
    // Allocate temporary register index for binary expression result
    int newTemp(int index1, int index2);
    // Allocate temporary register index for unary expression result
    int newTemp(int index);
    // Allocate temporary register index
    int newTemp();
    void setTemp(int n) {
        ntemps = n;
    }

    int tempCount() const {
        return ntemps;
    }
    void shrinkTemp() {
        ntemps = localSymbolCount();
    }

    void openScope() {
        scopes.push_back(SymbolScope());
    }

    void closeScope() {
        scopes.pop_back();
        shrinkTemp();
    }

private:
    // Function name
    string name;
    // Function codes
    std::vector<Code> codes;
    // Opcodes' line number
    std::vector<int>lines;
    // Constants in function
    std::vector<Operand> constants;
    // Count of parameters
    int nparams;
    // Count of return values
    int nresults;
    // Count of registers used
    int nslots;
    // Children functions
    std::vector<Function *> children;
    // Local symbol scopes
    std::vector<SymbolScope> scopes;
    // Upvalues
    std::vector<UpvalueInfo> upvalueInfos;
    // Temporaries
    int ntemps;
    // Parent function
    Function *parent;
};

// Upvalues for closures
struct Upvalue {
    bool isopen;
    union {
        int index;      // the stack index (when open)
        Operand value;  // the value (when closed)
    };

    Upvalue(): isopen(false) {}
    ~Upvalue() {}
};

// All runtime function are closures, this class object pointer to a
// prototype Function object and its upvalues.
class Closure {
public:
    Closure():prototype(nullptr) {
    }

    Closure(Function * prototype):prototype(prototype) {
    }

    Closure(const Closure &) = delete;
    Closure & operator = (const Closure &) = delete;

    Function *getPrototype() const {
        return prototype;
    }

    void setPrototype(Function * prototype) {
        this->prototype = prototype;
    }

    // Get upvalue by index
    int getUpvalueIndex(std::size_t i) const {
        return upvalueIndexes[i];
    }

    void addUpvalueIndex(int i) {
        upvalueIndexes.push_back(i);
    }

private:
    // Function prototype
    Function * prototype;
    // Upvalues indexes in runtime stack
    std::vector<int> upvalueIndexes;
};

#endif /* FUNCTION_H */
