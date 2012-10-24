#ifndef TMWA_SEXPR_PTR_HPP
#define TMWA_SEXPR_PTR_HPP
//    ptr.hpp - Safe smart pointers
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
    template<class T>
    class Unique;
    template<class T>
    class Shared;

    template<class T>
    class Unique
    {
        template<class U>
        friend class Unique;

        template<class U>
        friend class Shared;

        std::unique_ptr<T> impl;
    public:
        template<class... A>
        explicit Unique(A&&... a);

        Unique(Unique&&) = default;

        template<class U>
        Unique(Unique<U>);

        Unique& operator = (Unique&&) = default;

        template<class U>
        Unique& operator = (Unique<U>);

        T& operator *();

        const T& operator *() const;

        T *operator->();

        const T *operator->() const;
    };

    template<class T>
    class Shared
    {
        template<class U>
        friend class Shared;

        std::shared_ptr<T> impl;
    public:
        template<class... A>
        explicit Shared(A&&... a);

        Shared(Unique<T>);

        Shared(Shared&&) = default;

        Shared(const Shared&) = default;

        template<class U>
        Shared(Shared<U>);

        Shared& operator = (Unique<T>);

        Shared& operator = (Shared&&) = default;

        Shared& operator = (const Shared&) = default;

        template<class U>
        Shared& operator = (Shared<U>);

        T& operator *();

        const T& operator *() const;

        T *operator->();

        const T *operator->() const;
    };
} // namespace sexpr
} // namespace tmwa

#include "ptr.tcc"

#endif //TMWA_SEXPR_PTR_HPP
