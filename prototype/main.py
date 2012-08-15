#!/usr/bin/env python
# -*- encoding: utf-8 -*-

##    main.py - Dump representations of S-Expressions
##
##    Copyright © 2012 Ben Longbons <b.r.longbons@gmail.com>
##
##    This file is part of The Mana World (Athena server)
##
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import print_function, division

import sys
import os

import parser

def do_parser(p):
    for expr in p:
        print('%s' % expr)

if len(sys.argv) == 1:
    print('Parsing /dev/stdin')
    do_parser(parser.Parser('/dev/stdin', sys.stdin))
else:
    for arg in sys.argv[1:]:
        print('Parsing %s' % arg)
        do_parser(parser.Parser(arg))

