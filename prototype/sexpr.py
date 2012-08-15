#!/usr/bin/env python
# -*- encoding: utf-8 -*-

##    sexpr.py - Structures to represent S-Expressions
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

from abc import ABCMeta, abstractmethod


import linked_list


__all__ = ('Expression', 'List', 'Integer', 'String', 'Token', 'from_python')

class Expression(object):
    __metaclass__ = ABCMeta
    __slots__ = ('line', 'column')

    def __init__(self, line, column):
        self.line = line
        self.column = column

    @abstractmethod
    def __repr__(self):
        raise NotImplementedError

    @abstractmethod
    def __str__(self):
        raise NotImplementedError
# class Expression

class List(Expression):
    __slots__ = ('value',)
    def __init__(self, line, column, value):
        Expression.__init__(self, line, column)
        self.value = value

    def __repr__(self):
        return 'sexpr.List(%d, %d, %r)' % (self.line, self.column, self.value)

    def __str__(self):
        return '(' + ' '.join((str(item) for item in self.value)) + ')'

    def __iter__(self):
        return self.value.__iter__()

def _escape(ch, for_string):
    if for_string is False and ch == ' ':
        return "\\ "
    if ch in ("'", '"', '\\'):
        return "\\" + ch
    if ch >= ' ' and ch <= '~':
        return ch
    if ch == '\n':
        return "\\n"
    if ch == '\r':
        return "\\r"
    if ch == '\t':
        return "\\t"
    if ch == '\a':
        return "\\a"
    if ch == '\b':
        return "\\b"
    if ch == '\x1b':
        return "\\e"
    if ch == '\v':
        return "\\v"
    if ch == '\f':
        return "\\f"
    return "\\x%02x" % ord(ch)

class String(Expression):
    __slots__ = ('value',)
    def __init__(self, line, column, value):
        Expression.__init__(self, line, column)
        self.value = value

    def __repr__(self):
        return "sexpr.String(%d, %d, %r)" % (self.line, self.column, self.value)

    def __str__(self):
        return '"' + "".join( (_escape(ch, True) for ch in self.value) ) + '"'

class Int(Expression):
    __slots__ = ('value',)
    def __init__(self, line, column, value):
        Expression.__init__(self, line, column)
        self.value = value

    def __repr__(self):
        return 'sexpr.Int(%d, %d, %d)' % (self.line, self.column, self.value)

    def __str__(self):
        return str(self.value)

    # Will this be representable in an int64_t ?
    # Used by the parser
    def _okay(self):
        return -0x8000000000000000 <= self.value and self.value <= 0x7fffffffffffffff

class Token(Expression): # Includes ints when returned from Lexer; never empty
    def __init__(self, line, column, value):
        self.line = line
        self.column = column
        self.value = value

    def __repr__(self):
        return "sexpr.Token(%d, %d, %r)" % (self.line, self.column, self.value)

    def __str__(self):
        return "".join( (_escape(ch, False) for ch in self.value) )

    # Used by the parser
    def _maybe_int(self):
        try:
            # Note: int() may return a python long
            return Int(self.line, self.column, int(self.value, 0))
        except ValueError:
            return self

def from_python(expr):
    """ Use python syntax to build SExprs

        Unfortunately, you can't omit the commas :P.
    """
    if isinstance(expr, Expression):
        return expr
    if isinstance(expr, basestring):
        return String(0, 0, str(expr))
    if isinstance(expr, int) or isinstance(expr, long):
        return Int(0, 0, expr)
    # No tokens at this level
    list = linked_list.List()
    for elt in expr:
        list.append(from_python(elt))
    return List(0, 0, list)

