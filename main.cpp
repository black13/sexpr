//    main.cpp - For now, just check Variant a bit.
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

#include "sexpr.hpp"
#include "parser.hpp"
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

    void script()
    {
        Environment env = create_new_environment();
        Parser parser(TrackingStream("/dev/stdin"));
        SExpr sex;
        do
        {
            sex = parser.next();
            ValuePtr val = compile(env, sex).eval(env);
            if (val)
                std::cout << val->repr() << std::endl;
            // else
            //     std::cout << "()" << std::endl;
        }
        while (std::cout);
        std::cout << '\n';
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
            script();
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
