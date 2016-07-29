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

void concat(Semantic * front1, Semantic * front2)
{
    if(!front1 || !front2)
        throw "Invalid semantic info list connection";

    auto back1 = front1->prev;
    auto back2 = front2->prev;
    front1->prev = back2;
    front2->prev = back1;
    back1->next = front2;
    back2->next = front1;
}

// Delete doubly linked list
void destroy(Semantic * front)
{
    auto back0 = front->prev; // old back, to be delete
    auto back1 = back0->prev; // new back
    while(back0 != front) {
        // disconnect old back, set new back
        back1->next = front;
        front->prev = back1;
        delete back0;

        back0 = front->prev;
        back1 = back0->prev;
    }
    delete back0;
}

// Count results
int count(Semantic * front)
{
    int n = 1;
    auto p = front->next;
    while(p != front) {
        ++n;
        p = p->next;
    }
    return n;
}
