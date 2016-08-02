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

#include "VM.h"
#include <iostream>

VM::VM(Function *mfunction): mfunction(mfunction)
{
    // Initialize registers
    registers.resize(MINIMUM_REGISTER_SIZE);
    registers.resize(mfunction->slotCount() + 1);
    // Create closure for main function
    auto closure = createClosure(mfunction);
    registers[0] = Operand(closure);
    // Create superior caller
    calls.push_back(CallInfo(0, 1, 1, mfunction->getBaseCode()));
}

void VM::run()
{
    try {
        std::cout << "----------BEGIN OF PROGRAM----------\n";
        showRuntimeStack();
        while (!calls.empty()) {
            std::cout << "\n-----JUMP/RETURN TO SUBROUTINE-----\n";
            showRuntimeStack();
            bool finish = false;
            while (!finish) {
                auto function = getCurrentClosure()->getPrototype();
                auto baseCode = function->getBaseCode();

                Code::OpCode op = calls.back().pc->op;
                int arg1 = calls.back().pc->arg1;
                int arg2 = calls.back().pc->arg2;
                int result = calls.back().pc->result;

                calls.back().pc++;

                std::cout << "\nOP:" << opdesc[op]
                             << "\targ1:" << arg1
                             << "\targ2:" << arg2
                             << "\tresult:" << result << std::endl;

                switch (op) {
                case Code::Add:
                    R(result) = RK(arg1) + RK(arg2);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Sub:
                    R(result) = RK(arg1) - RK(arg2);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Mul:
                    R(result) = RK(arg1) * RK(arg2);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Div:
                    R(result) = RK(arg1) / RK(arg2);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Pow:
                    R(result) = pow(RK(arg1), RK(arg2));
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Minus:
                    R(result) = -RK(arg1);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Jmp:
                    calls.back().pc = baseCode + result;
                    break;
                case Code::Jnz:
                    if (!(RK(arg1).isFalse()))
                        calls.back().pc = baseCode + result;
                    break;
                case Code::Jgt:
                    if (RK(arg1) > RK(arg2))
                        calls.back().pc = baseCode + result;
                    break;
                case Code::Move:
                    R(result) = RK(arg1);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Closure:
                    R(result) = Operand(createClosure(getCurrentClosure()->getPrototype()->getChild(arg1)));
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::SetUpval:
                    U(result) = RK(arg1);
                    break;
                case Code::GetUpval:
                    R(result) = U(arg1);
                    calls.back().adjustTopIndex(result);
                    break;
                case Code::Call:
                    callClosure(arg1, arg2, result);
                    break;
                case Code::Return:
                    callReturn(arg1, arg2);
                    finish = true;
                    break;
                case Code::Nil:
                    for(int i = arg1; i < arg2; ++i)
                        R(i).setNil();
                    calls.back().adjustTopIndex(arg1 + arg2 - 1);
                    break;
                default:
                    throw "Invalid opcode";
                    break;
                } // switch
                if(!calls.empty())
                    showRuntimeStack();
            } // while
        } // while
    }
    catch(const char *msg) {
        std::cout << msg << std::endl;
    }
}

// CALL A B C -- R(A), ... ,R(A+C-1) = R(A)(R(A+1), ... ,R(A+B))
// wherein, A -- i, B -- nparams, C -- nresults
void VM::callClosure(int i, int nparams, int nresults)
{
    if(R(i).type != Operand::ClosureType)
        throw "Call a non-closure type";

    auto function = R(i).closure->getPrototype();
    auto code = function->getBaseCode();
    registers.resize(calls.back().topIndex + function->slotCount());
    //std::cout << "capacity:" << registers.capacity() << std::endl;

    int closureIndex = calls.back().baseIndex + i;
    int baseIndex = closureIndex + 1;
    int topIndex = calls.back().topIndex;
    calls.back().adjustTopIndex(i + nresults - 1);
    calls.push_back(CallInfo(closureIndex, baseIndex, topIndex, code));
}

Closure *VM::createClosure(Function * function)
{
    auto closure = new Closure(function);
    closures.push_back(closure);

    // setup upvalues
    auto count = function->upvalueCount();
    for (std::size_t i = 0; i < count; ++i) {
        auto upvalueInfo = function->getUpvalueInfo(i);
        if (upvalueInfo->isParentLocal) {
            closure->addUpvalue(calls.back().baseIndex + upvalueInfo->registerIndex);
        } else {
            // Get upvalue from parent upvalue list
            closure->addUpvalue(getCurrentClosure()->getUpvalue(upvalueInfo->registerIndex));
        }
    }
    return closure;
}

const Closure *VM::getClosure(std::size_t i) const
{
    return closures[i];
}

// RETURN A B - -- RETRUN R(A), ... ,R(A+B-1)
// wherein, A -- start, B -- n
void VM::callReturn(int start, int n)
{
    int closureIndex = calls.back().closureIndex;
    for(int i = 0; i < n; ++i) {
        registers[closureIndex+i] = R(start+i);
    }
    calls.pop_back();
    // Set nils
    if(calls.empty()) return;
    int topIndex = calls.back().topIndex;
    for(int i = closureIndex+n; i < topIndex; i++)
        registers[i].setNil();
}

void VM::showRuntimeStack() const
{
    std::cout <<"========RUNTIME STACK========" << std::endl;
    std::cout << "Closure Index:" << calls.back().closureIndex
              << "\tBase Index:" << calls.back().baseIndex
              << "\tTop Index:" << calls.back().topIndex << std::endl;
    int i = 0;
    for (auto a: registers) {
        if( i >= calls.back().topIndex)
            break;

        if(i == calls.back().baseIndex)
            std::cout << "base->";
        else if(i == calls.back().closureIndex)
            std::cout << "func->";
        std::cout << "\t" << a << std::endl;
        i++;
    }
    std::cout <<"=============================" << std::endl;
}
