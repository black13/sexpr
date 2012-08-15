#!/usr/bin/env python
# -*- encoding: utf-8 -*-

##    stream.py - A stream that with useful error info
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

from collections import namedtuple

__all__ = ('Unexpected', 'TrackingStream')

PositionInfo = namedtuple('PositionInfo', ['filename', 'line', 'column', 'text'])

class Unexpected(SyntaxError):
    __slots__ = ()

    def __init__(self, pos, msg):
        if isinstance(pos, TrackingStream):
            pos = pos.position_info()
        SyntaxError.__init__(self, msg, pos)
    # function __init__
# class Unexpected

class TrackingStream(object):
    __slots__ = ('_filename', '_line', '_col', '_text', '_input', '_eof_message')

    def __init__(self, name, input=None):
        if input is None:
            input = open(name)
        self._filename = name
        self._line = 0
        self._col = 0
        # self._text is initialized by _next_line()
        self._input = input
        self._eof_message = ''

        self._next_line()
        if self._text.startswith('#!'):
            # Might be EOF, but who cares?
            self._next_line()
    # function __init__

    def position_info(self):
        return PositionInfo(self._filename, self._line, self._col + 1, self._text)

    def on_eof(self, msg):
        self._eof_message = msg

    def off_eof(self):
        self.on_eof('')

    def _next_line(self):
        nl = self._input.readline()
        self._text = nl
        self._line += 1
        self._col = 0

        if nl == '' and self._eof_message != "":
            raise Unexpected(self, _self.eof_message)
        for c in nl:
            if c < ' ' and c != '\n':
                if c == '\t':
                    raise Unexpected(self, 'tab (try the "expand" program)')
                if c == '\r':
                    raise Unexpected(self, 'carriage return (try the "dos2unix" program)')
                raise Unexpected(self, 'C0 control character')
    # function _next_line

    def get(self):
        return self._text[self._col] if self._col < len(self._text) else ''

    def advance(self):
        self._col += 1
        if self._col == len(self._text):
            self._next_line()

    def advance_and_get(self):
        self.advance()
        return self.get()

    def get_and_advance(self):
        s = self.get()
        self.advance()
        return s
