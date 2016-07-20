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

#include "Operand.h"
#include "Function.h"
#include "math.h"

void Operand::copy(const Operand & a)
{
    this->type = a.type;
    switch (a.type) {
    case Operand::NilType:
        this->type = Operand::NilType;
        break;
    case Operand::ClosureType:
        this->closure = a.closure;
        break;
    case Operand::RealType:
        this->real = a.real;
        break;
    case Operand::IntegerType:
        this->integer = a.integer;
        break;
    default:
        break;
    }
}

Operand::Operand(const Operand & a)
{
    copy(a);
}

Operand & Operand::operator =(const Operand & a)
{
    copy(a);
    return *this;

}

ostream & operator <<(ostream & os, const Operand & a)
{
    switch (a.type) {
    case Operand::NilType:
        os << "Nil";
        break;
    case Operand::ClosureType:
        os << "Closure:" << a.closure <<" [Proto:" << a.closure->getPrototype() << "]";
        break;
    case Operand::RealType:
        os << a.real;
        break;
    case Operand::IntegerType:
        os << a.integer;
        break;
    default:
        os << "Nil";
        break;
    }
    return os;
}

bool operator ==(const Operand & left, const Operand & right)
{
    if (left.type != right.type)
        return false;

    switch (left.type) {
    case Operand::NilType:
        return true;
    case Operand::ClosureType:
        return left.closure == right.closure;
    case Operand::RealType:
        return left.real == right.real;
    case Operand::IntegerType:
        return left.integer == right.integer;
    default:
        throw "Unkown operand type";
    }
}

bool operator !=(const Operand & left, const Operand & right)
{
    return !(left == right);
}

bool operator >(const Operand & left, const Operand & right)
{
    Operand result;
    if (left.type == Operand::IntegerType
            && right.type == Operand::IntegerType) {
        return left.integer > right.integer;
    } else if (left.type == Operand::IntegerType
               && right.type == Operand::RealType) {
        return left.integer > right.real;
    } else if (left.type == Operand::RealType
               && right.type == Operand::IntegerType) {
        return left.real > right.integer;
    } else if (left.type == Operand::RealType
               && right.type == Operand::RealType) {
        return left.real > right.real;
    } else {
        throw "invalid operands type in comparison";
    }
}

Operand operator +(const Operand & left, const Operand & right)
{
    Operand result;
    if (left.type == Operand::IntegerType
            && right.type == Operand::IntegerType) {
        result.type = Operand::IntegerType;
        result.integer = left.integer + right.integer;
    } else if (left.type == Operand::IntegerType
               && right.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = left.integer + right.real;
    } else if (left.type == Operand::RealType
               && right.type == Operand::IntegerType) {
        result.type = Operand::RealType;
        result.real = left.real + right.integer;
    } else if (left.type == Operand::RealType
               && right.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = left.real + right.real;
    } else {
        throw "Invalid operands type in addition";
    }
    return result;
}

Operand operator -(const Operand & left, const Operand & right)
{
    Operand result;
    if (left.type == Operand::IntegerType
            && right.type == Operand::IntegerType) {
        result.type = Operand::IntegerType;
        result.integer = left.integer - right.integer;
    } else if (left.type == Operand::IntegerType
               && right.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = left.integer - right.real;
    } else if (left.type == Operand::RealType
               && right.type == Operand::IntegerType) {
        result.type = Operand::RealType;
        result.real = left.real - right.integer;
    } else if (left.type == Operand::RealType
               && right.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = left.real - right.real;
    } else {
        throw "Invalid operands type in substraction";
    }
    return result;
}

Operand operator *(const Operand & left, const Operand & right)
{
    Operand result;
    if (left.type == Operand::IntegerType
            && right.type == Operand::IntegerType) {
        result.type = Operand::IntegerType;
        result.integer = left.integer * right.integer;
    } else if (left.type == Operand::IntegerType
               && right.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = left.integer * right.real;
    } else if (left.type == Operand::RealType
               && right.type == Operand::IntegerType) {
        result.type = Operand::RealType;
        result.real = left.real * right.integer;
    } else if (left.type == Operand::RealType
               && right.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = left.real * right.real;
    } else {
        throw "Invalid operands type in multiplication";
    }
    return result;
}

Operand operator /(const Operand & left, const Operand & right)
{
    Operand result;
    double a, b;
    if (left.type == Operand::IntegerType
            && right.type == Operand::IntegerType) {
        a = double(left.integer);
        b = double(right.integer);
    } else if (left.type == Operand::IntegerType
               && right.type == Operand::RealType) {
        a = double(left.integer);
        b = right.real;
    } else if (left.type == Operand::RealType
               && right.type == Operand::IntegerType) {
        a = left.real;
        b = double(right.integer);
    } else if (left.type == Operand::RealType
               && right.type == Operand::RealType) {
        a = left.real;
        b = right.real;
    } else {
        throw "Invalid operands type in division";
    }

    result.type = Operand::RealType;
    result.real = a/b;
    return result;
}

Operand pow(const Operand & left, const Operand & right)
{
    Operand result;
    double a, b;
    if (left.type == Operand::IntegerType
            && right.type == Operand::IntegerType) {
        a = double(left.integer);
        b = double(right.integer);
    } else if (left.type == Operand::IntegerType
               && right.type == Operand::RealType) {
        a = double(left.integer);
        b = right.real;
    } else if (left.type == Operand::RealType
               && right.type == Operand::IntegerType) {
        a = left.real;
        b = double(right.integer);
    } else if (left.type == Operand::RealType
               && right.type == Operand::RealType) {
        a = left.real;
        b = right.real;
    } else {
        throw "Invalid operands type in pow";
    }

    result.type = Operand::RealType;
    result.real = pow(a, b);
    return result;
}

Operand operator -(const Operand & a) {
    Operand result;
    if (a.type == Operand::IntegerType) {
        result.type = Operand::IntegerType;
        result.integer = -a.integer;
    } else if (a.type == Operand::RealType) {
        result.type = Operand::RealType;
        result.real = -a.real;
    } else {
        throw "Invalid operands type in unary minus";
    }

    return result;
}
