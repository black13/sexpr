#ifndef TMWA_SEXPR_IO_HPP
#define TMWA_SEXPR_IO_HPP
//    io.hpp - iostream interfaces for SExprs.
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

#include <ostream>

#include "sexpr.hpp"

namespace tmwa
{
namespace sexpr
{
    std::ostream& operator << (std::ostream&, const SExpr&);
#if 0
    std::istream& operator >> (std::istream&, SExpr&);
#endif
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_IO_HPP
