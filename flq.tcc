//    flq.tcc - implementation of inlines and templates in flq.hpp
//
//    Copyright © 2012 Ben Longbons <b.r.longbons@gmail.com>
//
//    This file is part of The Mana World (Athena server)
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdlib>

namespace tmwa
{
namespace sexpr
{
    template<class T>
    void flq<T>::push_back(T t)
    {
        if (this->empty())
        {
            this->push_front(std::move(t));
            return;
        }
        // TODO implement this properly
        auto it = this->begin();
        // assert it != end
        auto prev = it;
        while (++it != this->end())
            prev = it;
        // assert it == end && std::next(prev) == it
        this->insert_after(prev, std::move(t));
    }
} // namespace sexpr
} // namespace tmwa
