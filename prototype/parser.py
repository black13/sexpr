#!/usr/bin/env python
# -*- encoding: utf-8 -*-

##    parser.py - Parse a stream of text into a sequence of SExprs
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
from cStringIO import StringIO

from sexpr import Expression, List, Int, String, Token
from stream import TrackingStream, Unexpected

import linked_list

__all__ = ('Parser', 'parse_string')

def warn(s):
    print('Warning:', s, file=sys.stderr)

# Logically, these aren't expressions, but it's convenient to have a single superclass.
class BeginList(Expression):
    """ Marker for a list's opening parenthesis
    """

    __slots__ = ()

    def __init__(self, line, column):
        Expression.__init__(self, line, column)

    def __repr__(self):
        return 'parser.BeginList(%d, %d)' % (self.line, self.column)

    def __str__(self):
        return '('
# class BeginList

class EndList(Expression):
    """ Marker for a list's closing parenthesis
    """

    __slots__ = ()

    def __init__(self, line, column):
        Expression.__init__(self, line, column)

    def __repr__(self):
        return 'parser.EndList(%d, %d)' % (self.line, self.column)

    def __str__(self):
        return ')'
# class EndList

class Lexer(object):
    """ An iterator over the lexemes of an SExpr stream.

        Lexemes are: BeginList, EndList, String, Token
    """
    __slots__ = ('_source', '_depth')
    def __init__(self, stream):
        assert isinstance(stream, TrackingStream)
        self._source = stream
        self._depth = [] # Positions of unmatched ( so far
        # (_depth could be a plain integer counter, but this gives debug info)

    def __iter__(self):
        return self

    def next(self):
        self._source.off_eof()
        # First, skip whitespace
        ch = ' ' # really a do-while
        while ch in (' ', '\n'):
            pos = self._source.position_info()
            ch = self._source.get_and_advance()
            if not ch: # EOF
                if self._depth:
                    loc = self._depth[-1]
                    raise Unexpected(loc, "unmatched '('")
                else:
                    raise StopIteration
        # Whitespace skipper

        if ch == '(':
            self._depth.append(pos)
            return BeginList(pos.line, pos.column)
        if ch == ')':
            if not self._depth:
                raise Unexpected(pos, "unmatched ')'")
            self._depth.pop()
            return EndList(pos.line, pos.column)
        if ch == '"':
            s = bytearray() # Quotes are not part of a string literal
            self._source.on_eof("EOF in string literal")
            while True:
                ch = self._source.get_and_advance()
                if ch == '"':
                    # Quotes are not part of a string literal
                    return String(pos.line, pos.column, str(s))
                if ch != '\\':
                    s += ch
                    continue
                self._source.on_eof("EOF in backslash sequence in string literal")
                s += self._read_after_backslash()
                self._source.on_eof("EOF in string literal")
            # while True
        # if ch == '"'

        # to get here, it's either a token or an integer
        # Note: EOF is off
        # Note: we have one character in ch, for which we have already .advance()ed,
        # but we must not have advanced in order to return without munging the (, ", and ) cases
        # Thus, the copy-pasta
        #tok = ''
        if ch != '\\':
            tok = ch
        else: # ch == '\\'
            self._source.on_eof("EOF in backslash sequence in token")
            tok = self._read_after_backslash()
            self._source.off_eof()
        # read until whitespace or EOF
        while True:
            ch = self._source.get()
            # Note: if we could do the first iteration, these first conditions are guaranteed not to happen
            if ch == "" or ch == ' ' or ch == '\n':
                return Token(pos.line, pos.column, tok)
            if ch in ('(', '"', ')'):
                warn("some parsers may have difficulty with a %r attached to a token" % ch)
                return Token(pos.line, pos.column, tok)
            self._source.advance()
            if ch != '\\':
                tok += ch
            else: # ch == '\\'
                self._source.on_eof("EOF in backslash sequence in token")
                tok += self._read_after_backslash()
                self._source.off_eof()
        # while True
    # next()

    def _read_after_backslash(self):
        # Note: EOF throwing must be set before calling this!
        ch = self._source.get_and_advance()
        if ch == ' ' or ch == '\n':
            return ch
        if ch in ('"', "'", '\\'):
            return ch
        if ch == 'n':
            return '\n'
        if ch == 'r':
            return '\r'
        if ch == 't':
            return '\t'
        if ch == 'a':
            return '\a'
        if ch == 'b':
            return '\b'
        if ch == 'e':
            return '\x1b'
        if ch == 'v':
            return '\v'
        if ch == 'f':
            return '\f'
        if ch == 'x':
            ch1 = self._source.get_and_advance()
            ch2 = self._source.get_and_advance()
            return chr(int(ch1 + ch2, 16))
        if ch in '01234567':
            oct_str = ch
            ch2 = self._source.get()
            if ch2 in '01234567':
                self._source.advance()
                oct_str += ch2
                ch3 = self.source.get()
                if ch3 in '01234567':
                    self._source.advance()
                    oct_str += ch3
                # else ch3 will be read again from the outer loop by self.source.get_and_advance
            # else ch2 will be read again from the outer loop by self.source.get_and_advance
            return chr(int(oct_str, 8))
        raise Unexpected(self.source, "Character following backslash: " + repr(ch))
# class Lexer

class Parser(object):
    """ Takes the lexer and makes a data tree.

        There are two significant things that happen here:
        1. Turn BeginList(), ..., EndList() into a List
        2. Turn some Token into Int
    """

    __slots__ = ('_lexer',)

    def __init__(self, name, input=None):
        self._lexer = Lexer(TrackingStream(name, input))

    def __iter__(self):
        return self

    def next(self):
        # The lexer already takes care of ( vs ) matching
        lx = self._lexer.next()
        assert type(lx) in (BeginList, EndList, String, Token)
        if not isinstance(lx, BeginList):
            if isinstance(lx, Token):
                return lx._maybe_int()
            return lx
        line = lx.line
        column = lx.column
        lst = linked_list.List()
        while True:
            # Not self._lexer.next() - we recurse
            lx = self.next()
            if isinstance(lx, EndList):
                return List(line, column, lst)
            lst.append(lx)
# class Parser

def parse_string(s, n='None'):
    parser = Parser(n, StringIO(s))
    rv, = parser
