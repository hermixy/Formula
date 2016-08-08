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

#ifndef CODE_H
#define CODE_H

#include <string>
using std::string;

// instructions description
static const string opdesc[] = {
    "NOP",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "POW",
    "MINUS",
    "MOD",

    "JMP",
    "JNZ",
    "JLT",
    "JLE",
    "JGT",
    "JGE",
    "JEQ",
    "JNE",

    "MOVE",

    "CLOSURE",

    "SETUPVAL",
    "GETUPVAL",

    "CALL",
    "RETURN",

    "NIL",

    "FORPREP",
    "FORLOOP",
};

struct Code {
    // instructions set
    enum OpCode {
        Nop,		/* - - - -- used as a placeholder */
        Add,		/* A B C -- R(C) = RK(A)+RK(B) */
        Sub,		/* A B C -- R(C) = RK(A)-RK(B) */
        Mul,		/* A B C -- R(C) = RK(A)*RK(B) */
        Div,		/* A B C -- R(C) = RK(A)/RK(B) */
        Pow,		/* A B C -- R(C) = RK(A)^RK(B) */
        Minus,		/* A - C -- R(C) = -RK(A) */
        Mod,		/* A B C -- R(C) = RK(A)%RK(B) */

        Jmp,		/* - - C -- PC += C */
        Jnz,		/* A - C -- if(RK(A) == true PC = C */
        Jlt,		/* A B C -- if(RK(A) < RK(B)) PC = C */
        Jle,		/* A B C -- if(RK(A) <= RK(B)) PC = C */
        Jgt,		/* A B C -- if(RK(A) > RK(B)) PC = C */
        Jge,		/* A B C -- if(RK(A) >= RK(B)) PC = C */
        Jeq,		/* A B C -- if(RK(A) == RK(B)) PC = C */
        Jne,		/* A B C -- if(RK(A) != RK(B)) PC = C */

        Move,		/* A - C -- R(C) = RK(A) */

        Closure,	/* A - C -- create closure with the A-th function */

        SetUpval,	/* A - C -- U(C) = RK(A) */
        GetUpval,	/* A - C -- R(C) = U(A) */

        Call,		/* A B C -- R(A), ... ,R(A+C-1) = R(A)(R(A+1), ... ,R(A+B)) */
        Return,		/* A B - -- return R(A), ... ,R(A+B-1) */

        Nil,        /* A B - -- R(A), ..., R(A+B-1) = Nil */

        ForPrep,    /* A - C -- */
        ForLoop,    /* A - C -- */
    };

    // three-address code
    OpCode op;
    int arg1;
    int arg2;
    int result;

    Code():op(Code::Nop) {
    }

    Code(OpCode op, int arg1, int arg2, int result): op(op), arg1(arg1),
        arg2(arg2), result(result) {
    }
};

#endif /* CODE_H */
