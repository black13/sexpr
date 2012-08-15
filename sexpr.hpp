#ifndef TMWA_SEXPR_SEXPR_HPP
#define TMWA_SEXPR_SEXPR_HPP
//    sexpr.hpp - Basic data structures used for S-expressions.
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

#include <string>

#include "flq.hpp"
#include "variant.hpp"

namespace tmwa
{
namespace sexpr
{
    class SExpr;
    class List : public flq<SExpr>
    {
    public:
        template<class... A>
        List(A&&... a)
        : flq<SExpr>(std::forward<A>(a)...)
        {}
    };

    class Int
    {
        // arbitrarily signed rather than unsigned
        // will cause shorter to_string conversion
    public:
        int64_t value;
        Int(int64_t v = 0)
        : value(v)
        {}
    };

    class String : public std::string
    {
    public:
        template<class... A>
        String(A&&... a)
        : std::string(std::forward<A>(a)...)
        {}
    };

    class Token : public std::string
    {
    public:
        template<class... A>
        Token(A&&... a)
        : std::string(std::forward<A>(a)...)
        {}
    };

    class SExpr : public Variant<Void, List, Int, String, Token>
    {
    public:
        SExpr(Void = Void()) {}
        SExpr(List l) { emplace<List>(l); }
        SExpr(Int i) { emplace<Int>(i); }
        SExpr(String s) { emplace<String>(s); }
        SExpr(Token t) { emplace<Token>(t); }
    };
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_SEXPR_HPP
