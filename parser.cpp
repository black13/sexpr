#include "parser.hpp"
//    parser.cpp - Parse an S-expression from an istream.
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

namespace tmwa
{
namespace sexpr
{
    char Lexer::read_after_backslash()
    {
        char c = *source++;
        switch(c)
        {
        case ' ':
        case '\n':
        case '"':
        case '\'':
        case '\\':
        case '(':
        case ')':
            return c;
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'e':
            return '\e';
        case 'v':
            return '\v';
        case 'f':
            return '\f';
        case 'x':
            {
                char c1 = *source++;
                if ('0' <= c1 && c1 <= '9')
                    c1 -= '0';
                else if ('A' <= c1 && c1 <= 'F')
                    c1 -= 'A' - '0';
                else if ('a' <= c1 && c1 <= 'f')
                    c1 -= 'a' - '0';
                else
                    throw Unexpected(source.position(), "nonhex digit");
                char c2 = *source++;
                if ('0' <= c2 && c2 <= '9')
                    c2 -= '0';
                else if ('A' <= c2 && c2 <= 'F')
                    c2 -= 'A' - '0';
                else if ('a' <= c2 && c2 <= 'f')
                    c2 -= 'a' - '0';
                else
                    throw Unexpected(source.position(), "nonhex digit");
                return c1 * 16 + c2;
            }
        case '0' ... '7':
            {
                c -= '0';
                char c2 = *source;
                if ('0' <= c2 && c2 <= '7')
                {
                    c2 -= '0';
                    ++source;
                    char c3 = *source;
                    if ('0' <= c3 && c3 <= '7')
                    {
                        c3 -= '0';
                        return c * 64 + c2 * 8 + c3;
                    }
                    else
                    {
                        // c3 will be reread in future
                        return c * 8 + c2;
                    }
                }
                else
                {
                    // c2 will be reread in future
                    return c;
                }
            }
        default:
            throw Unexpected(source.position(), "character following backslash");
        }
    }

    Lexeme Lexer::next()
    {
        source.off_eof();
        char ch;
        Position pos;
        do
        {
            if (!source)
            {
                if (!depth.empty())
                    throw Unexpected(depth.back(), "unmatched '('");
                else
                    return EndOfStream();
            }
            pos = source.position();
            ch = *source++;
        }
        while (ch == ' ' || ch == '\n');
        // whitespace has been skipped
        if (ch == '(')
        {
            depth.push_back(pos);
            return BeginList();
        }
        if (ch == ')')
        {
            if (depth.empty())
                throw Unexpected(pos, "unmatched ')'");
            depth.pop_back();
            return EndList();
        }
        if (ch == '"')
        {
            std::string s;
            source.on_eof("EOF in string literal");
            while (true)
            {
                ch = *source++;
                if (ch == '"')
                    return String(s);
                if (ch != '\\')
                {
                    s += ch;
                    continue;
                }
                source.on_eof("EOF in backslash sequence in string literal");
                s += read_after_backslash();
                source.on_eof("EOF in string literal");
            }
        }

        // if we get here, it's either a token or an integer
        // (they are distinguished by the parser)
        // (yes, this means that \x30 is a valid integer)
        std::string tok;
        if (ch != '\\')
            tok += ch;
        else
        {
            source.on_eof("EOF in backslash sequence in token");
            tok += read_after_backslash();
            source.off_eof();
        }

        // EOF is fine in a token
        while (source)
        {
            char ch = *source;
            if (ch == ' ' || ch == '\n')
            {
                return Token(tok);
            }

            if (ch == '(' || ch == '"' || ch == ')')
            {
                // used to give a warning about being hard to parse
                // removed since I'm the only one parsing and I solved it
                return Token(tok);
            }

            ++source;

            if (ch != '\\')
                tok += ch;
            else
            {
                source.on_eof("EOF in backslash sequence in token");
                tok += read_after_backslash();
                source.off_eof();
            }
        }
        return Token(tok); // hit EOF
    }

    class MaybeEndList
    {
        List *out;
    public:
        MaybeEndList(List *l) : out(l) {}
        bool operator () (Void) { return false; }
        template<class T>
        bool operator () (T t) { out->push_back(t); return true; }
    };

    class ApplyMaybeList
    {
        Parser *parser;
    public:
        ApplyMaybeList(Parser *p)
        : parser(p)
        {}
        SExpr operator () (EndOfStream)
        {
            return Void();
        }
        SExpr operator () (BeginList bl)
        {
            // size_t line = bl.line;
            // size_t column = bl.column;
            List out;
            bool keep_going = true;
            while (keep_going)
            {
                SExpr nx = parser->next();
                apply(keep_going, MaybeEndList(&out), nx);
            }
            return out;
        }
        SExpr operator () (EndList el)
        {
            return Void();
        }
        SExpr operator () (String s)
        {
            return s;
        }
        SExpr operator () (Token t)
        {
            const char *cstr = t.value.c_str();
            char *end;
            errno = 0;
            long long l = strtoll(cstr, &end, 0);
            switch(errno)
            {
            default:
                abort();
            case 0:
            case EINVAL:
                break;
            case ERANGE:
                throw Unexpected(Position{"<unknown>", 0, 0, ""}, "out of range int");
            }
            if (end - cstr != t.value.size())
                return t;
            return Int(l);
        };
    };

    SExpr Parser::next()
    {
        // publically, returns an SExpr containing Void on eof.
        // Internally, also returns a Void to handle end-of-list.
        // This is safe because the Lexer balances the parentheses for us.
        Lexeme lx = lexer.next();

        SExpr out;
        apply(out, ApplyMaybeList(this), lx);
        return out;
    }
} // namespace sexpr
} // namespace tmwa
