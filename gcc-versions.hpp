#ifndef TMWA_SEXPR_GCC_VERSIONS_HPP
#define TMWA_SEXPR_GCC_VERSIONS_HPP
//    gcc-versions.hpp - Work around old compiler versions.
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

#if __GNUC__ < 4
# error "Really old compiler!"
#endif
#if __GNUC__ == 4 and not (defined __clang__)
# if __GNUC_MINOR__ < 7
#  define override /* empty */
#  define final /* empty */
# endif
#endif

#endif //TMWA_SEXPR_GCC_VERSIONS_HPP
