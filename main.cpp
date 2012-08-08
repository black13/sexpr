//    main.cpp - For now, just check Variant a bit.
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

#include "variant.hpp"

#include <string>
#include <iostream>

namespace tmwa
{
namespace sexpr
{
} // namespace sexpr
} // namespace tmwa

class Print
{
public:
    template<class T>
    void operator () (T&& t)
    {
        std::cout << std::move(t) << std::endl;
    }
};

int main()
{
    tmwa::sexpr::Variant<int, std::string> v;
    tmwa::sexpr::apply(tmwa::sexpr::Void(), Print(), v);
}
