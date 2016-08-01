
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

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <iostream>
#include <string>
using std::string;

static const char *desc[] = {
    "Constant",
    "Identifier",
    "Expression",
    "FunctionCall",
    "FunctionDefinition",
    "LocalSymbol",
    "Upvalue",
};

struct SemanticInfo {
    enum SemanticType {
        Constant,
        Identifier,
        Expression,
        FunctionCall,
        FunctionDefinition,
        LocalSymbol,
        Upvalue,
    };

    SemanticType type;
    int index;
    int codeIndex;
    string name;		// identifier

    SemanticInfo(SemanticType type, int index)
        : type(type), index(index) {
        std::cout << "++ Create semantic information: " << this << " (" << desc[type] << ")" << std::endl;
    }

    SemanticInfo(SemanticType type, int index, int codeIndex)
        : type(type), index(index), codeIndex(codeIndex) {
        std::cout << "++ Create semantic information: " << this << " (" << desc[type] << ")" << std::endl;
    }

    SemanticInfo(SemanticType type, string name)
        : type(type), name(name) {
        std::cout << "++ Create semantic information: " << this << " (" << desc[type] << ")" << std::endl;
    }

    ~SemanticInfo() {
        std::cout << "-- Delete semantic information: " << this << std::endl;
    }
};

// Doubly linked list node
struct Semantic {
    SemanticInfo *info;
    Semantic *prev;
    Semantic *next;

    Semantic(SemanticInfo *info): info(info) {
        prev = this;
        next = this;
    }
    ~Semantic() {
        delete info;
    }
    Semantic(const Semantic &) = delete;
    Semantic & operator = (const Semantic &) = delete;

    // Connect two doubly linked list, front1 is the header of the new list
    friend void concat(Semantic * front1, Semantic * front2);
    // Delete doubly linked list
    friend void destroy(Semantic * front);
    // Count results
    friend int count(Semantic * front);
};



#endif /* SEMANTIC_H */
