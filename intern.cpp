#include "intern.hpp"
//    intern.cpp - std::string pool, assuming cow
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

#include <set>

namespace tmwa
{
namespace sexpr
{
    static std::set<std::string> pool;

    std::string intern(const char * cs)
    {
        // TODO: use a second std::map because pointer compare is cheaper than string compare
        return intern(std::string(cs));
    }

    std::string intern(std::string s)
    {
        // TODO rewrite using .lower_bound() or .upper_bound()
        // or maybe even .equal_range()
        auto it = pool.find(s);
        if (it != pool.end())
            return *it;
        pool.insert(s);
        return s;
    }

    void deintern(const std::string& s)
    {
        auto it = pool.find(s);
        if (it != pool.end())
            pool.erase(it);
    }

    void deintern_all()
    {
        pool.clear();
    }
} // namespace sexpr
} // namespace tmwa
