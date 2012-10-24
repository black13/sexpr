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

namespace tmwa
{
namespace sexpr
{
    class NilValue : public BaseValue
    {
    public:
        virtual SExpr repr() override
        {
            return List();
        }
    };

    ValuePtr nil = Shared<NilValue>();

    Evaluable eval_to_nil = [](Environment&, Continuation ret)
    {
        ret(nil);
    };

    void warn(const std::string& s)
    {
        std::cout << "Warning: " << s << std::endl;
    }

    void Evaluable::eval(Environment& env, Continuation c) const
    {
        (*impl)(env, c);
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

                Evaluable out;
                // TODO: see if it's possible to CPS_ify this
                // (at present it throws if out is not initialized)
                // This would allow crazy code like:
                // ((block (sleep 2) foo) "foo args")
                // which is roughly equivalent to:
                // (block (sleep 2) (foo "foo args"))
                compile(*env, l.take_front()).eval(*env,
                    [&out, &l, this](ValuePtr vp)
                    {
                        CallablePtr func = vp->as_callable();
                        out = (*func)(*env, std::move(l));
                    }
                );
                return out;
            }
            Evaluable operator()(Int i)
            {
                ValuePtr vp = Shared<IntValue>(i.value);
                return [vp] (Environment&, Continuation ret)
                {
                    ret(vp);
                };
            }
            Evaluable operator()(String s)
            {
                ValuePtr vp = Shared<StringValue>(std::move(s.value));
                return [vp] (Environment&, Continuation ret)
                {
                    ret(vp);
                };
            }
            Evaluable operator()(Token t)
            {
                std::string varname = std::move(t.value);
                return [varname] (Environment& env, Continuation ret)
                {
                    auto it = env.find(varname);
                    if (it == env.end())
                        ret(nil);
                    else
                        ret(it->second);
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

            // called when the script-level function is called at this site
            void operator()(Environment& env, Continuation ret) const
            {
                Shared<flq<ValuePtr>> args;
                Evaluable er = [this, args](Environment& env, Continuation ret)
                {
                    ret ((*impl)(env, *args));
                };

                for (auto it = eargs.rbegin(), end = eargs.rend(); it != end; ++it)
                {
                    // is it really necessary to stack them 2 deep?
                    Evaluable earg = *it;
                    er = [er, earg, args](Environment& env, Continuation ret)
                    {
                        earg.eval(env, [&env, args, er, ret](ValuePtr vp)
                        {
                            args->push_back(vp);
                            // er has its own binding to args
                            er.eval(env, ret);
                        });
                    };
                }

                er.eval(env, ret);
            }
        };
    public:
        FunctionCallable(RealFunction rf)
        : impl(Shared<RealFunction>(std::move(rf)))
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

            void operator()(Environment& env, Continuation ret)
            {
                cond.eval(env, [this, ret, &env](ValuePtr c)
                {
                    if (c->as_int())
                        if_true.eval(env, ret);
                    else
                        if_false.eval(env, ret);
                });
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
            return [varname, rhs] (Environment& env, Continuation ret)
            {
                rhs.eval(env, [&env, varname, ret](ValuePtr tmp)
                {
                    auto pair = env.insert({varname, tmp});
                    if (!pair.second)
                        pair.first->second = tmp;
                    ret(nil);
                });
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
            std::cout << vp->repr() << std::endl;
        }
        return nil;
    };

    ValuePtr builtin_function(Environment&, flq<ValuePtr>);

    std::map<std::string, Callable> builtins =
    {
        {"if", {"if", Shared<CallableImpl>(ConditionalCallable())}},
        {"let", {"let", Shared<CallableImpl>(AssignmentCallable())}},
        {"print", {"print", Shared<CallableImpl>(FunctionCallable(print_function))}},
        {"builtin", {"builtin", Shared<CallableImpl>(FunctionCallable(builtin_function))}},
    };

    ValuePtr builtin_function(Environment&, flq<ValuePtr> q)
    {
        if (q.empty())
            throw ScriptError("missing builtin argument");
        std::string name = q.take_front()->as_string();
        if (!q.empty())
            throw ScriptError("extra builtin garbage");
        return Shared<Callable>(builtins.at(name));
    }

    Environment create_new_environment()
    {
        return Environment
        {
            {"builtin", Shared<Callable>(builtins.at("builtin"))},
        };
    }
} // namespace sexpr
} // namespace tmwa
