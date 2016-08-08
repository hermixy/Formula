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

#ifndef OPERAND_H
#define OPERAND_H

#include <iostream>
using std::ostream;

class Closure;

class Operand {
public:
    enum OperandType {
        NilType,
        ClosureType,
        RealType,
        IntegerType,
    };

    union {
        Closure *closure;
        double real;
        int integer;
    };

    OperandType type;

    Operand():type(Operand::NilType) {
    }

    ~Operand() {
        // Operand object only holds closure pointer, the deallocation of closure object
        // is implemented in class VM
    }

    explicit Operand(Closure * closure):closure(closure),
        type(Operand::ClosureType) {
    }

    explicit Operand(double real):real(real), type(Operand::RealType) {
    }

    explicit Operand(int integer):integer(integer),
        type(Operand::IntegerType) {
    }

    Operand(const Operand & a);

    void setNil() {
        type = Operand::NilType;
    }

    bool isNil() const {
        return type == Operand::NilType;
    }

    bool isFalse() const {
        return type == Operand::NilType || (type == Operand::IntegerType
                                            && integer == 0);
    }

    Operand & operator =(const Operand & a);

    friend bool operator ==(const Operand & left, const Operand & right);
    friend bool operator !=(const Operand & left, const Operand & right);
    friend bool operator >(const Operand & left, const Operand & right);
    friend bool operator <(const Operand & left, const Operand & right);
    friend bool operator >=(const Operand & left, const Operand & right);
    friend bool operator <=(const Operand & left, const Operand & right);
    friend Operand operator +(const Operand & left, const Operand & right);
    friend Operand operator -(const Operand & left, const Operand & right);
    friend Operand operator *(const Operand & left, const Operand & right);
    friend Operand operator /(const Operand & left, const Operand & right);
    friend Operand pow(const Operand & left, const Operand & right);
    friend Operand operator -(const Operand & a);

    friend ostream & operator <<(ostream & os, const Operand & a);

private:
    void copy(const Operand & a);
};

#endif /* OPERAND_H */
