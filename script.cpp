#include "script.hpp"
//    script.cpp - Implement a basic script engine
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

#include <vector>

#include "io.hpp"
#include "intern.hpp"

namespace tmwa
{
namespace sexpr
{
    ValuePtr nil;
    Evaluable eval_to_nil = [](Environment&)
    {
        return nil;
    };

    void warn(const std::string& s)
    {
        std::cout << "Warning: " << s << std::endl;
    }

    ValuePtr Evaluable::eval(Environment& env)
    {
        return (*impl)(env);
    }

    Evaluable compile(Environment& env, SExpr code)
    {
        class Compiler
        {
            Environment *env;
        public:
            Compiler(Environment *e)
            : env(e)
            {}

            Evaluable operator()(List l)
            {
                if (l.empty())
                    return eval_to_nil;
                // This does not prevent the development of lambdas - just pass the bound variables in the environment at call time.
                // however, it does prevent nonconstant function pointers
                CallablePtr func = compile(*env, l.take_front()).eval(*env)->as_callable();
                return (*func)(*env, std::move(l));
            }
            Evaluable operator()(Int i)
            {
                ValuePtr vp = std::make_shared<IntValue>(i.value);
                return [vp] (Environment&)
                {
                    return vp;
                };
            }
            Evaluable operator()(String s)
            {
                ValuePtr vp = std::make_shared<StringValue>(std::move(s.value));
                return [vp] (Environment&)
                {
                    return vp;
                };
            }
            Evaluable operator()(Token t)
            {
                std::string varname = std::move(t.value);
                return [varname] (Environment& env)
                {
                    return env[varname];
                };
            }
            Evaluable operator()(Void)
            {
                throw std::logic_error("attempt to compile eof!");
            }
        };
        Evaluable out;
        apply(out, Compiler(&env), code);
        return out;
    }

    class FunctionCallable
    {
        RealFunctionPtr impl;

        struct FunctionFunctor
        {
            RealFunctionPtr impl;
            std::vector<Evaluable> eargs;

            ValuePtr operator()(Environment& env)
            {
                // TODO replace this with (delayable) CPS
                flq<ValuePtr> args;
                for (Evaluable& arg : eargs)
                    args.push_back(arg.eval(env));
                return (*impl)(env, std::move(args));
            }
        };
    public:
        FunctionCallable(RealFunction rf)
        : impl(std::make_shared<RealFunction>(std::move(rf)))
        {}

        Evaluable operator()(Environment& env, List args)
        {
            std::vector<Evaluable> eargs;
            eargs.reserve(std::distance(args.begin(), args.end()));
            for (SExpr& sexpr : args)
                eargs.push_back(compile(env, sexpr));
            return FunctionFunctor{impl, std::move(eargs)};
        }
    };

    class ConditionalCallable
    {
        class ConditionalFunctor
        {
            Evaluable cond, if_true, if_false;
        public:
            ConditionalFunctor(Evaluable c, Evaluable t, Evaluable f)
            : cond(std::move(c))
            , if_true(std::move(t))
            , if_false(std::move(f))
            {}

            ValuePtr operator()(Environment& env)
            {
                ValuePtr c = cond.eval(env);
                return (c && c->as_int()) ? if_true.eval(env) : if_false.eval(env);
            }
        };
    public:
        Evaluable operator()(Environment& env, List args)
        {
            if (args.empty())
                throw ScriptError("missing if cond");
            Evaluable cond = compile(env, args.take_front());
            if (args.empty())
                throw ScriptError("missing if iftrue");
            Evaluable if_true = compile(env, args.take_front());
            if (args.empty())
                return ConditionalFunctor(std::move(cond), std::move(if_true), eval_to_nil);
            Evaluable if_false = compile(env, args.take_front());
            if (!args.empty())
                throw ScriptError("extra if arguments");
            return ConditionalFunctor(std::move(cond), std::move(if_true), std::move(if_false));
        }
    };

    class AssignmentCallable
    {
    public:
        Evaluable operator()(Environment& env, List args)
        {
            if (args.empty())
                throw ScriptError("missing let varname");
            Token var;
            class GetIfToken
            {
            public:
                Token operator ()(Token t) { return t; }
                void operator ()(Void) {}
            };
            apply(var, GetIfToken(), args.take_front());
            std::string varname = std::move(var.value);
            if (varname.empty())
                throw ScriptError("let varname not token");
            if (args.empty())
                throw ScriptError("missing let content");
            Evaluable rhs = compile(env, args.take_front());
            if (!args.empty())
                throw ScriptError("extra let garbage");
            return [varname, rhs] (Environment& env) mutable
            {
                env[varname] = rhs.eval(env);
                return nil;
            };
        }
    };

    ValuePtr print_function(Environment&, flq<ValuePtr> q)
    {
        bool first = true;
        for (ValuePtr& vp : q)
        {
            if (!first)
                std::cout << ' ';
            else
                first = false;
            SExpr sexpr;
            if (vp)
                sexpr = vp->repr();
            std::cout << sexpr << std::endl;
        }
        return nil;
    };

    ValuePtr builtin_function(Environment&, flq<ValuePtr>);

    std::map<std::string, Callable> builtins =
    {
        {intern("if"), {intern("if"), std::make_shared<CallableImpl>(ConditionalCallable())}},
        {intern("let"), {intern("let"), std::make_shared<CallableImpl>(AssignmentCallable())}},
        {intern("print"), {intern("print"), std::make_shared<CallableImpl>(FunctionCallable(print_function))}},
        {intern("builtin"), {intern("builtin"), std::make_shared<CallableImpl>(FunctionCallable(builtin_function))}},
    };

    ValuePtr builtin_function(Environment&, flq<ValuePtr> q)
    {
        if (q.empty())
            throw ScriptError("missing builtin argument");
        std::string name = q.take_front()->as_string();
        if (!q.empty())
            throw ScriptError("extra builtin garbage");
        return std::make_shared<Callable>(builtins.at(name));
    }

    Environment create_new_environment()
    {
        std::string builtin = intern("builtin");
        return Environment
        {
            {builtin, std::make_shared<Callable>(builtins.at(builtin))},
        };
    }
} // namespace sexpr
} // namespace tmwa
