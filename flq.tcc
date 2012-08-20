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
    flq<T>::operator bool()
    {
        return !impl.empty();
    }

    template<class T>
    bool flq<T>::empty()
    {
        return impl.empty();
    }

    template<class T>
    T& flq<T>::front()
    {
        return impl.front();
    }

    template<class T>
    const T& flq<T>::front() const
    {
        return impl.front();
    }

    template<class T>
    T flq<T>::take_front()
    {
        T old_front = std::move(impl.front());
        pop_front();
        return old_front;
    }

    template<class T>
    void flq<T>::pop_front()
    {
        impl.pop_front();
        if (impl.empty())
            // we just popped the only element, it must have been *last
            last = impl.before_begin();
    }

    template<class T>
    T& flq<T>::back()
    {
        return *last;
    }

    template<class T>
    const T& flq<T>::back() const
    {
        return *last;
    }

    template<class T>
    void flq<T>::push_back(T v)
    {
        last = impl.insert_after(last, std::move(v));
    }

    template<class T>
    template<class... A>
    void flq<T>::emplace_back(A&&... a)
    {
        last = impl.emplace_after(last, std::forward<A>(a)...);
    }

    template<class T>
    typename flq<T>::iterator flq<T>::begin()
    {
        return impl.before_begin();
    }

    template<class T>
    typename flq<T>::iterator flq<T>::end()
    {
        return last;
    }

    template<class T>
    typename flq<T>::const_iterator flq<T>::begin() const
    {
        return impl.before_begin();
    }

    template<class T>
    typename flq<T>::const_iterator flq<T>::end() const
    {
        return last;
    }
} // namespace sexpr
} // namespace tmwa
