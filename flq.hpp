#ifndef TMWA_SEXPR_FLQ_HPP
#define TMWA_SEXPR_FLQ_HPP
//    flq.hpp - Appendable singly-linked list.
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

#include <forward_list>
#include <cstddef>

namespace tmwa
{
namespace sexpr
{
    // a forward_list stores only the front "pointer"
    // this wrapper stores the back "pointer" as well
    template<class T>
    class flq
    {
        typedef std::forward_list<T> _list;
        typedef typename _list::iterator _iterator;
        _list impl;
        _iterator last;
    public:
        flq() : impl(), last(impl.before_begin()) {}
        // copy/move construct and assign are NOT quite fine, but destruct is (of course)
        flq(flq&&);
        flq(const flq&);
        flq& operator = (flq);
        template<class It>
        flq(It b, It e) : impl(), last(impl.before_begin())
        {
            while (b != e)
            {
                push_back(*b);
                ++b;
            }
        }
        flq(std::initializer_list<T> l) : impl(), last(impl.before_begin())
        {
            for (auto b = l.begin(), e = l.end(); b != e; ++b)
                push_back(*b);
        }

        explicit operator bool();
        bool empty();
        T& front();
        const T& front() const;
        T take_front();
        void pop_front();
        T& back();
        const T& back() const;
        void push_back(T v);
        template<class... A>
        void emplace_back(A&&... a);

        class iterator;
        class const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
    };

    template<class T>
    flq<T>::flq(flq&& r)
    : impl(std::move(r.impl))
    , last(r.last)
    {
        if (last == r.impl.before_begin())
            last = impl.before_begin();
    }

    template<class T>
    flq<T>::flq(const flq& r) : impl(), last(impl.before_begin())
    {
        for (auto b = r.begin(), e = r.end(); b != e; ++b)
            push_back(*b);
    }

    template<class T>
    flq<T>& flq<T>::operator = (flq<T> r)
    {
        impl = std::move(r.impl);
        last = r.last;
        if (last == r.impl.before_begin())
            last = impl.before_begin();
        return *this;
    }

    template<class T>
    class flq<T>::iterator
    {
        typedef std::forward_list<T> _list;
        typedef typename _list::iterator _iterator;
        _iterator impl;

        friend class flq<T>::const_iterator;
    public:
        iterator() : impl() {}
        iterator(_iterator it) : impl(it) {}

        typedef T value_type;
        typedef T *pointer;
        typedef T& reference;
        typedef ptrdiff_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        reference operator *() const
        {
            return *std::next(impl);
        }

        pointer operator->() const
        {
            return std::addressof(*(*this));
        }

        iterator& operator ++()
        {
            ++impl;
            return *this;
        }

        iterator operator++ (int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator == (iterator l, iterator r)
        {
            return l.impl == r.impl;
        }

        friend bool operator != (iterator l, iterator r)
        {
            return !(l == r);
        }
    };

    template<class T>
    class flq<T>::const_iterator
    {
        typedef std::forward_list<T> _list;
        typedef typename _list::iterator _iterator;
        typedef typename _list::const_iterator _const_iterator;
        _const_iterator impl;
        typedef typename flq<T>::iterator iterator;
    public:
        const_iterator() : impl() {}
        const_iterator(iterator it) : impl(it.impl) {}
        const_iterator(_iterator it) : impl(it) {}
        const_iterator(_const_iterator it) : impl(it) {}

        typedef T value_type;
        typedef const T *pointer;
        typedef const T& reference;
        typedef ptrdiff_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        reference operator *() const
        {
            return *std::next(impl);
        }

        pointer operator->() const
        {
            return std::addressof(*(*this));
        }

        const_iterator& operator ++()
        {
            ++impl;
            return *this;
        }

        const_iterator operator++ (int)
        {
            const_iterator tmp = *this;
            ++impl;
            return tmp;
        }

        friend bool operator == (const_iterator l, const_iterator r)
        {
           return l.impl == r.impl;
        }

        friend bool operator != (const_iterator l, const_iterator r)
        {
           return !(l == r);
        }

        friend bool operator == (iterator l, const_iterator r)
        {
            return const_iterator(l) == r;
        }

        friend bool operator != (iterator l, const_iterator r)
        {
            return !(l == r);
        }

        friend bool operator == (const_iterator l, iterator r)
        {
            return l == const_iterator(r);
        }

        friend bool operator != (const_iterator l, iterator r)
        {
            return !(l == r);
        }
    };
} // namespace sexpr
} // namespace tmwa

#include "flq.tcc"

#endif //TMWA_SEXPR_FLQ_HPP
