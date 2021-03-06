#ifndef TMWA_SEXPR_VOID_HPP
#define TMWA_SEXPR_VOID_HPP
//    void.hpp - A type that represents nothing and anything.
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

namespace tmwa
{
namespace sexpr
{
    struct Void
    {
        template<class T>
        constexpr operator T() noexcept { return T(); }
        template<class T>
        void operator = (T&&) noexcept {}
        template<class T>
        constexpr Void(T&&) noexcept {}
        constexpr Void() noexcept;
    };
    constexpr Void::Void() noexcept = default;
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_VOID_HPP
