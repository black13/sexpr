#ifndef TMWA_SEXPR_SCRIPT_HPP
#define TMWA_SEXPR_SCRIPT_HPP
//    script.hpp - Compile and run S-Expressions.
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
#include <map>
#include <stdexcept>
#include <iostream>

#include "sexpr.hpp"
#include "ptr.hpp"

namespace tmwa
{
namespace sexpr
{
    class ScriptError : public std::exception
    {
        std::string msg;
    public:
        ScriptError(std::string s)
        : msg(std::move(s))
        {}

        virtual const char *what() const noexcept override
        {
            return msg.c_str();
        }
    };

    class BaseValue;
    class Evaluable;
    typedef Shared<BaseValue> ValuePtr;
    typedef std::map<std::string, ValuePtr> Environment;
    typedef Shared<Evaluable> EvaluablePtr;
    typedef std::function<void(ValuePtr)> Continuation;
    typedef std::function<void(Environment&, Continuation)> EvaluableImplFunction;
    typedef Shared<EvaluableImplFunction> EvaluableImplFunctionPtr;
    typedef std::function<ValuePtr(Environment&, flq<ValuePtr>)> RealFunction;
    typedef Shared<RealFunction> RealFunctionPtr;
    typedef std::function<Evaluable(Environment&, List)> CallableImpl;
    typedef Shared<CallableImpl> CallablePtr;

    void warn(const std::string&);

    class BaseValue
    {
    public:
        virtual int64_t as_int() { warn("Not integer"); return 0; }
        virtual std::string as_string() { warn("Not string"); return std::string(); }
        virtual CallablePtr as_callable() { /* no warn - handled elsewhere */ return CallablePtr(); }
        virtual SExpr repr() = 0;
        virtual ~BaseValue() {}
    };

    class Evaluable
    {
        EvaluableImplFunctionPtr impl;
    public:
        // You might wonder why I'm doing this, instead of just taking a
        // std::function directly. The answer is that that would require
        // a double conversion, which doesn't happen. This way only
        // requires a single conversion.
        template<class F>
        Evaluable(F&& f)
        : impl(Shared<EvaluableImplFunction>(std::forward<F>(f)))
        {}

        Evaluable() = default;
        Evaluable(const Evaluable&) = default;
        Evaluable(Evaluable&&) = default;
        Evaluable(Evaluable&);
        Evaluable& operator = (const Evaluable&) = default;
        Evaluable& operator = (Evaluable&&) = default;

        void eval(Environment&, Continuation) const;
    };
    inline Evaluable::Evaluable(Evaluable&) = default;

    Evaluable compile(Environment& env, SExpr code);

    class Callable : public BaseValue
    {
        CallablePtr impl;
        std::string name;
    public:
        Callable(std::string n, CallablePtr p)
        : impl(std::move(p))
        , name(std::move(n))
        {}
        CallablePtr as_callable() override { return impl; }
        SExpr repr() override { return List({ Token("builtin"), String(name)}); }
    };

    class IntValue : public BaseValue
    {
        int64_t value;
    public:
        IntValue(int64_t v = 0)
        : value(v)
        {}
        int64_t as_int() override { return value; }
        SExpr repr() override { return Int(value); }
    };

    class StringValue : public BaseValue
    {
        std::string value;
    public:
        StringValue(std::string s = std::string())
        : value(std::move(s))
        {}
        std::string as_string() override { return value; }
        SExpr repr() override { return String(value); }
    };

    // environment will contain only "builtin"
    Environment create_new_environment();
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_SCRIPT_HPP
