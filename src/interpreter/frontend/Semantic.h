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

struct SemanticInfo {
	enum SemanticType {
		Constant,
		Identifier,
		Expression,
	};

	SemanticType type;
	int index;
	string name;		// identifier
	SemanticInfo *next;

	SemanticInfo(SemanticType type, int index, SemanticInfo *next) 
	: type(type), index(index), next(next) {
		std::cout << "++ Create semantic information: " << this << std::endl;
	}

	SemanticInfo(SemanticType type, string name, SemanticInfo *next) 
	: type(type), name(name), next(next) {
		std::cout << "++ Create semantic information: " << this << std::endl;
	}

	~SemanticInfo() {
		std::cout << "-- Delete semantic information: " << this << std::endl;
		delete next;
	}

	friend SemanticInfo * merge(SemanticInfo *info1, SemanticInfo *info2); 
};

class Semantic {

};

#endif /* SEMANTIC_H */
