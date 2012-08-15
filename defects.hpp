#ifndef TMWA_SEXPR_DEFECTS_HPP
#define TMWA_SEXPR_DEFECTS_HPP
//    defects.hpp - Supply things the standard should have.
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

#include <memory>

namespace tmwa
{
namespace sexpr
{
    template<class T, class... A>
    std::unique_ptr<T> make_unique(A&&... a)
    {
        return std::unique_ptr<T>(new T(std::forward<A>(a)...));
    }
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_DEFECTS_HPP
