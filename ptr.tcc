//    ptr.tcc - implementation of inlines and templates in ptr.hpp
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
    template<class B, class D>
    constexpr bool _is_safe_base_of()
    {
        return std::has_virtual_destructor<B>::value && std::is_base_of<B, D>::value;
    };

    template<class B, class D, typename=typename std::enable_if<_is_safe_base_of<B,D>()>::type>
    void _must_be_safe_base()
    {}

    template<class T>
    template<class... A>
    Unique<T>::Unique(A&&... a)
    : impl(new T(std::forward<A>(a)...))
    {}

#if +0
    template<class T>
    Unique<T>::Unique(Unique<T>&&) = default;
#endif

    template<class T>
    Unique<T>::Unique(const Unique<T>&&) = default;

    template<class T>
    template<class U>
    Unique<T>::Unique(Unique<U> u)
    : impl(std::move(u.impl))
    {
        _must_be_safe_base<T, U>();
    }

#if +0
    template<class T>
    Unique<T>::Unique(Unique<T>&&) = default;
#endif

    template<class T>
    template<class U>
    Unique<T>& Unique<T>::operator = (Unique<U> u)
    {
        _must_be_safe_base<T, U>();
        impl = std::move(u.impl);
        return *this;
    }

    template<class T>
    T& Unique<T>::operator *() const
    {
        return *impl;
    }

    template<class T>
    T *Unique<T>::operator->() const
    {
        return &*impl;
    }

    template<class T>
    template<class... A>
    Shared<T>::Shared(A&&... a)
    : impl(std::make_shared<T>(std::forward<A>(a)...))
    {}

    template<class T>
    Shared<T>::Shared(Unique<T> u)
    : impl(std::move(u.impl))
    {}

#if +0
    template<class T>
    Shared<T>::Shared(Shared&&) = default;
#endif

    template<class T>
    Shared<T>::Shared(const Shared&&) = default;

    template<class T>
    Shared<T>::Shared(Shared&) = default;

#if +0
    template<class T>
    Shared<T>::Shared(const Shared&) = default;
#endif

    template<class T>
    template<class U>
    Shared<T>::Shared(Shared<U> su)
    : impl(std::move(su.impl))
    {}

    template<class T>
    Shared<T>& Shared<T>::operator = (Unique<T> u)
    {
        impl = std::move(u.impl);
        return *this;
    }

#if +0
    template<class T>
    Shared<T>& Shared<T>::operator = (Shared<T>&& s) = default;

    template<class T>
    Shared<T>& Shared<T>::operator = (const Shared<T>& s) = default;
#endif

    template<class T>
    template<class U>
    Shared<T>& Shared<T>::operator = (Shared<U> su)
    {
        impl = std::move(su.impl);
        return *this;
    }

    template<class T>
    T& Shared<T>::operator *() const
    {
        return *impl;
    }

    template<class T>
    T *Shared<T>::operator ->() const
    {
        return &*impl;
    }
} // namespace sexpr
} // namespace tmwa
