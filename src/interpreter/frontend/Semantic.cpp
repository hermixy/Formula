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

#include "Semantic.h"

SemanticInfo * merge(SemanticInfo *info1, SemanticInfo *info2)
{
	SemanticInfo *rear = info1; 
	if(!info1 || !info2) 
		throw "Invalid semantic information merge";
	
	while(rear->next)
		rear = rear->next;

	rear->next = info2;

	return info1;
}
