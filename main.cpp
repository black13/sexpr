//    main.cpp - For now, just check Variant a bit.
//
//    Copyright © 2012-2014 Ben Longbons <b.r.longbons@gmail.com>
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

#include "sexpr.hpp"
#include "parser.hpp"
#include "ptr.hpp"
#include "io.hpp"
#include "script.hpp"

#include <string>
#include <iostream>

namespace tmwa
{
namespace sexpr
{
    void echo()
    {
        Parser parser(TrackingStream("/dev/stdin"));
        SExpr sex;
        do
        {
            sex = parser.next();
            std::cout << sex << std::endl;
        }
        while (std::cout);
        std::cout << std::endl;
    }

    void help()
    {
        std::cout << "pass one argument" << std::endl;
        std::cout << "known arguments: help, echo, script, list, sexpr" << std::endl;
    }

    void script_inner_loop(bool interactive, Environment& env, Parser& parser, std::function<void(void)>& resume)
    {
        SExpr sex = parser.next();
        if (sex.is<Void>())
            return;
        compile(env, sex).eval(env, [interactive, &env, &parser, &resume](Shared<Value> val)
        {
            if (interactive)
                std::cout << val->repr() << std::endl;
            // in reality, resume would have a timeout and be closely related to Environment
            resume = [interactive, &env, &parser, &resume]()
            {
                script_inner_loop(interactive, env, parser, resume);
            };
        });
    }

    void script(bool interactive)
    {
        Environment env = create_new_environment();
        Parser parser(TrackingStream("/dev/stdin"));
        std::function<void(void)> resume = [interactive, &env, &parser, &resume]()
        {
            script_inner_loop(interactive, env, parser, resume);
        };
        while (true)
        {
            std::function<void(void)> current;
            std::swap(resume, current);
            if (!current)
                break;
            current();
        }
        std::cout << '\n';
    }

    void ptr()
    {
        Unique<std::string> u(3, 'x');
        Unique<std::string> u2(std::move(u));
        Unique<std::istream> u3 = Unique<std::ifstream>("/dev/zero");
        u = std::move(u2);
        u3 = Unique<std::ifstream>("/dev/null");

        Shared<std::string> s(3, 'x');
        Shared<std::string> s2(std::move(s));
        Shared<std::istream> s3 = Shared<std::ifstream>("/dev/zero");
        s = std::move(s2);
        s3 = Shared<std::ifstream>("/dev/null");

        Shared<std::string> s4 = Unique<std::string>();
        s4 = std::move(u);
    }

    void match()
    {
        Variant<int, std::string> var;
        var.emplace<int>(42);
        for (int x = 0; x < 2; ++x)
        {
            MATCH (var)
            {
                CASE(int, i)
                {
                    std::cout << "integer: " << i << std::endl;
                }
                CASE(std::string, s)
                {
                    std::cout << "string: " << s << std::endl;
                }
            };
            if (x == 0)
                var.emplace<std::string>("hello");
        }
    }

    void main(std::string arg)
    {
        if (arg == "list")
        {
            flq<int> l = {1, 2, 3};
            l.push_back(4);
            l.push_back(5);
            for (int i : l)
                std::cout << i << ' ';
            std::cout << std::endl;
        }
        else if (arg == "sexpr")
        {
            SExpr v, l = List(), i = Int(), s = String("string"), t = Token("token");
            for (const SExpr& sex : {v, l, i, s, t})
                std::cout << sex << std::endl;
        }
        else if (arg == "echo")
        {
            echo();
        }
        else if (arg == "script")
        {
            script(false);
        }
        else if (arg == "shell")
        {
            script(true);
        }
        else if (arg == "ptr")
        {
            ptr();
        }
        else if (arg == "match")
        {
            match();
        }
        else
        {
            help();
        }
    }

} // namespace sexpr
} // namespace tmwa

int main(int argc, char **argv)
{
    if (argc == 2)
        tmwa::sexpr::main(argv[1]);
    else
        tmwa::sexpr::help();
}
