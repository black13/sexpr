#!/usr/bin/env python
# -*- encoding: utf-8 -*-

##    linked_list.py - A simple data structure supporting only front, append, push, and pop
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

class _Node(object):
    __slots__ = ('_data', '_next')
    def __init__(self, data, next = None):
        self._data = data
        self._next = next
    # function __init__
# class _Node

class List(object):
    __slots__ = ('_head', '_tail')
    def __init__(self, args=None):
        self._head = None
        self._tail = None
        if args is not None:
            for arg in args:
                self.append(arg)
    # function __init__

    def __nonzero__(self):
        return self._head is not None
    # function __nonzero__

    def __iter__(self):
        n = self._head
        while n is not None:
            yield n._data
            n = n._next
    # generator __iter__

    def __repr__(self):
        return 'linked_list.List(%s)' % ', '.join( (repr(e) for e in self) )
    # function __repr__

    def __str__(self):
        return '(%s)' % ' '.join( (str(e) for e in self) )
    # function __str__

    def empty(self):
        return self._head is None
    # function empty

    def front(self):
        if self.empty():
            raise IndexError()
        return self._head._data
    # function front

    def append(self, val):
        nn = _Node(val)
        if self.empty():
            self._head = nn
            self._tail = nn
            return
        old_tail = self._tail
        old_tail._next = nn
        self._tail = nn
    # function append

    def pop(self):
        if not self:
            raise IndexError()
        new_head = self._head._next
        if new_head is None:
            self._tail = None
        self._head = new_head
    # function pop

    def push(self, val):
        nn = _Node(val, self._head)
        if not self:
            self._tail = nn
        self._head = nn
    # function push

    def splice(self, other):
        if other._head is None:
            return
        if self._tail is None:
            self._head = other._head
            self._tail = other._tail
        else:
            self._tail._next = other._head
            self._tail = other._tail
        other._head = None
        other._tail = None
    # function splice

    def take_front(self):
        out = self.front()
        self.pop()
        return out
    # function take_front

    def swap(self, other):
        self._head, other._head = other._head, self._head
        self._tail, other._tail = other._tail, self._tail
    # function swap
# class List
