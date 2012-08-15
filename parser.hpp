#ifndef TMWA_SEXPR_PARSER_HPP
#define TMWA_SEXPR_PARSER_HPP
//    parser.hpp - Parse an S-expression from an istream.
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

#include <iterator>
#include <vector>

#include "tracking_stream.hpp"
#include "variant.hpp"
#include "sexpr.hpp"

namespace tmwa
{
namespace sexpr
{
    // just markers
    class BeginList {};
    class EndList {};
    class EndOfStream {};

    /// subclass instead of typedef just to get cleaner error messages
    class Lexeme : public Variant<EndOfStream, BeginList, EndList, String, Token>
    {
    public:
        template<class... A>
        Lexeme(A&&... a)
        : Variant<EndOfStream, BeginList, EndList, String, Token>(std::forward<A>(a)...)
        {}
    };

    /// Parse an character input stream into almost-iterator over lexemes.
    /// Also balances parentheses.
    class Lexer // : public std::iterator<std::input_iterator_tag, Lexeme>
    {
        TrackingStream source;
        // could be just an int, but this gives debug info
        std::vector<Position> depth;

        char read_after_backslash();
    public:
        Lexer(Lexer&&) = default;
        Lexer(TrackingStream in) : source(std::move(in)) {}
        Lexeme next();
    };

    /// Parse a lexeme stream into an an almost-iterator of SExpr trees
    class Parser
    {
        Lexer lexer;
    public:
        Parser(Parser&&) = default;
        Parser(Lexer l)
        : lexer(std::move(l))
        {}
        Parser(TrackingStream ts)
        : lexer(std::move(ts))
        {}
        SExpr next();
    };
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_PARSER_HPP
