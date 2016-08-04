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

#ifndef VM_H
#define VM_H

#include "Operand.h"
#include "Function.h"
#include <vector>
#include <list>

struct CallInfo {
    // Function/closure index int the stack
    int closureIndex;
    // Register base index(absolute) in the stack of current function
    int baseIndex;
    // Register top index(absolute) of current function
    int topIndex;
    // Program counter, i.e. current code
    Code *pc;

    CallInfo(): closureIndex(0), baseIndex(0), topIndex(0), pc(nullptr) {}

    CallInfo(int closureIndex, int baseIndex, int topIndex, Code *pc)
        : closureIndex(closureIndex), baseIndex(baseIndex), topIndex(topIndex), pc(pc){
    }

    // Adjust topIndex while running
    void adjustTopIndex(int i) {
        if(topIndex < baseIndex+i+1)
            topIndex = baseIndex+i+1;
    }
};

#define MINIMUM_REGISTER_SIZE 256

class VM {
public:
    VM(Function * mfunction);
    ~VM() {
        for(auto closure : closures)
            delete closure;
        for(auto upvalue : upvalues)
            delete upvalue;
    }

    // Execute the code
    void run();
    // Print runtime stack
    void showRuntimeStack() const;

private:
    // Call function/closure at register i(relative to current base index)
    void callClosure(int i, int nparams, int nresults);
    // Return values at register i(relative to current base index)
    void callReturn(int i, int n);

    // Register reference at index i(relative to base of current base index)
    Operand & R(std::size_t i) {
        return registers[calls.back().baseIndex + i];
    }

    // Register reference at index i(relative to base of current base index)
    const Operand & R(std::size_t i) const {
        return registers[calls.back().baseIndex + i];
    }

    // Register reference at index i(relative to base of current base index)
    // or the i-th constant of current function
    const Operand & RK(int i) const {
        if (i >= 0) {
            return R(i);
        } else {
            return getCurrentClosure()->getPrototype()->
                    getConstant(-i);
        }
    }

    // Upvalue reference at index i of current function/closure
    void setUpvalue(int i, const Operand &value) {
        i = getCurrentClosure()->getUpvalueIndex(i);
        if(upvalues[i]->isopen)
            registers[upvalues[i]->index] = value;
        else
            upvalues[i]->value = value;
    }

    // Upvalue reference at index i of current function/closure
    Operand getUpvalue(int i) const {
        i = getCurrentClosure()->getUpvalueIndex(i);
        if(upvalues[i]->isopen)
            return registers[upvalues[i]->index];
        else
            return upvalues[i]->value;
    }

    // Get current closure, i.e. activation record
    const Closure *getCurrentClosure() const {
        return registers[calls.back().closureIndex].closure;
    }

    // Create closure base on function
    Closure *createClosure(Function * function);

    // Closure at index i
    const Closure *getClosure(std::size_t i) const;

    // Get register base
    Operand *getBaseRegister() {
        return registers.empty() ? nullptr : &registers[0];
    }

    // Check whether upvalue already exisited
    int findUpvalue(int registerIndex);

    // Add upvalue
    int addUpvalue(int registerIndex);

    // Close upvalues assocciated to current closure
    void closeUpvalues();

    // Main fuction, starting point of the virtual machine
    Function *mfunction;
    // Runtime stack, registers of each function is one part of the stack.
    std::vector<Operand> registers;
    // Frame stack, informations of each function
    std::vector<CallInfo> calls;
    // Closures
    std::vector<Closure *> closures;
    // Upvalues
    std::vector<Upvalue *> upvalues;
};

#endif /* VM_H */
