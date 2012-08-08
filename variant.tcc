//    variant.tcc - implementation of inlines and templates in variant.hpp
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

#include <cassert>
#include "bind.hpp"

namespace tmwa
{
namespace sexpr
{
    struct Destruct
    {
        template<class U>
        void operator ()(U& v)
        {
            v.~U();
        }
    };

    template<class D, class... T>
    void Variant<D, T...>::do_destruct()
    {
        apply(Void(), Destruct(), *this);
    }

    template<class D, class... T>
    template<class C, class... A>
    void Variant<D, T...>::do_construct(A&&... a)
    {
        try
        {
            data.template construct<C, A...>(std::forward<A>(a)...);
            state = Union<D, T...>::template index<C>();
        }
        catch(...)
        {
            static_assert(std::is_nothrow_constructible<D>::value, "first element is nothrow constructible");
            data.template construct<D>();
            state = 0;
            throw;
        }
    }

    template<class D, class... T>
    Variant<D, T...>::Variant()
    {
        do_construct<D>();
        state = 0;
    }

    template<class D, class... T>
    Variant<D, T...>::~Variant()
    {
        do_destruct();
    }

    template<class D, class... T>
    void Variant<D, T...>::reset()
    {
        do_destruct();
        do_construct<D>();
    }

    template<class D, class... T>
    template<class C, class... A>
    void Variant<D, T...>::emplace(A&&... a)
    {
        do_destruct();
        do_construct<C, A...>(std::forward<A>(a)...);
    }

    template<class... T>
    class CopyConstruct
    {
        Variant<T...> *target;
    public:
        CopyConstruct(Variant<T...> *v) : target(v) {}
        template<class U>
        void operator ()(const U& u)
        {
            target->template do_construct<U, const U&>(u);
        }
    };
    template<class... T>
    class MoveConstruct
    {
        Variant<T...> *target;
    public:
        MoveConstruct(Variant<T...> *v) : target(v) {}
        template<class U>
        void operator ()(U&& u)
        {
            target->template do_construct<U, U>(std::move(u));
        }
    };

    // assignment requires unchecked access
    template<class... T>
    class CopyAssign
    {
        Union<T...> *data;
    public:
        CopyAssign(Union<T...> *d) : data(d) {}
        template<class U>
        void operator ()(const U& u)
        {
            *data->template get<U>() = u;
        }
    };

    template<class... T>
    class MoveAssign
    {
        Union<T...> *data;
    public:
        MoveAssign(Union<T...> *d) : data(d) {}
        template<class U>
        void operator () (U&& u)
        {
            *data->template get<U>() = std::move(u);
        }
    };

    template<class D, class... T>
    Variant<D, T...>::Variant(const Variant& r)
    {
        apply(Void(), CopyConstruct<D, T...>(this), r);
    }

    template<class D, class... T>
    Variant<D, T...>::Variant(Variant&& r)
    {
        apply(Void(), MoveConstruct<D, T...>(this), std::move(r));
    }

    template<class D, class... T>
    Variant<D, T...>& Variant<D, T...>::operator = (const Variant& r)
    {
        if (state == r.state)
            apply(Void(), CopyAssign<D, T...>(this), r);
        else
        {
            do_destruct();
            apply(Void(), CopyConstruct<D, T...>(this), r);
        }
        return *this;
    }

    template<class D, class... T>
    Variant<D, T...>& Variant<D, T...>::operator = (Variant&& r)
    {
        if (state == r.state)
            apply(Void(), MoveAssign<D, T...>(&data), std::move(r));
        else
        {
            do_destruct();
            apply(Void(), MoveConstruct<D, T...>(this), std::move(r));
        }
        return *this;
    }

    template<class R, class F>
    void _apply_assign(std::true_type, R& r, F&& f)
    {
        std::forward<F>(f)();
        r = Void();
    }

    template<class R, class F>
    void _apply_assign(std::false_type, R& r, F&& f)
    {
        r = std::forward<F>(f)();
    }

    template<class R, class F>
    void apply(R& r, F&& f)
    {
        _apply_assign(std::is_void<decltype(std::forward<F>(f)())>(), r, std::forward<F>(f));
    }

    class VariantFriend
    {
    public:
        template<class E, class... T>
        static E& unchecked_get(Variant<T...>& var)
        {
            return *var.data.template get<E>();
        }
        template<class E, class... T>
        static const E& unchecked_get(const Variant<T...>& var)
        {
            return *var.data.template get<E>();
        }
        template<class E, class... T>
        static E&& unchecked_get(Variant<T...>&& var)
        {
            return std::move(*var.data.template get<E>());
        }
        template<class E, class... T>
        static const E&& unchecked_get(const Variant<T...>&& var)
        {
            return std::move(*var.data.template get<E>());
        }

        template<class E, class R, class F, class V1, class... V>
        static void _apply_unchecked(R& r, F&& f, V1&& v1, V&&... v)
        {
            apply(r, bind_variadic(std::forward<F>(f), VariantFriend::unchecked_get<E>(std::forward<V1>(v1))), std::forward<V>(v)...);
        }

        template<class... T, class R, class F, class V1, class... V>
        static void _apply_dispatch(const Variant<T...> *, R& r, F&& f, V1&& v1, V&&... v)
        {
            typedef void (*Function)(R&, F&&, V1&&, V&&...);
            constexpr static Function dispatch[sizeof...(T)] = { _apply_unchecked<T, R, F, V1, V...>... };
            assert(v1.state < sizeof...(T));
            dispatch[v1.state](r, std::forward<F>(f), std::forward<V1>(v1), std::forward<V>(v)...);
        }
    };

    template<class R, class F, class V1, class... V>
    void apply(R& r, F&& f, V1&& v1, V&&... v)
    {
        VariantFriend::_apply_dispatch(&v1, r, std::forward<F>(f), std::forward<V1>(v1), std::forward<V>(v)...);
    }

    template<class F, class... V>
    void apply(Void&& r, F&& f, V&&... v)
    {
        apply(r, std::forward<F>(f), std::forward<V>(v)...);
    }

} // namespace sexpr
} // namespace tmwa
