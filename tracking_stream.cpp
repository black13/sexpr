#include "tracking_stream.hpp"
//    tracking_stream.cpp - istream wrapper that keeps track of position info
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
    Unexpected::Unexpected(const Position& pos, const std::string& msg)
    {
        std::ostringstream out;
        out << "At " << pos.filename << ':' << pos.line << ':' << pos.column << '\n';
        out << "Unexpected " << msg << '\n';
        if (!pos.line_contents.empty())
        {
            out << pos.line_contents; // contains a '\n'
            for (size_t i = 1; i < pos.column; ++i)
                out << ' ';
            out << '^';
        }
        message = out.str();
    }

    const char *Unexpected::what() const noexcept
    {
        return message.c_str();
    }

    FakeTrackingStream::FakeTrackingStream(char c)
    : datum(c)
    {}

    char FakeTrackingStream::operator *()
    {
        return datum;
    }

    void TrackingStream::next_line()
    {
        if (std::getline(*in, text))
            text += '\n';
        else
            text.clear();
        // hereafter empty text means error
        line++;
        col = 0;
        if (text.empty() and !eof_message.empty())
            throw Unexpected(position(), eof_message);
        for (unsigned char c : text)
            if (c < ' ' and c != '\n')
            {
                if (c == '\t')
                    throw Unexpected(position(), "tab (try the 'expand' program)");
                if (c == '\r')
                    throw Unexpected(position(), "carriage return (try the 'dos2unix' program)");
                throw Unexpected(position(), "C0 control character");
            }
    }

    TrackingStream::TrackingStream(std::string name, std::unique_ptr<std::istream> i)
    : in(std::move(i))
    , filename(std::move(name))
    , text() // initialized later
    , eof_message()
#if HANDLE_SHEBANG_SPECIALLY
    , shebang()
#endif
    , line(0)
    , col(0)
    {
        next_line();
#if HANDLE_SHEBANG_SPECIALLY
        if (0 == text.compare(0, 2, "#!"))
        {
            shebang = std::move(text);
            next_line();
        }
#endif
    }

    TrackingStream::TrackingStream(std::string name)
    : TrackingStream(name, make_unique<std::ifstream>(name))
    {
    }

#ifdef HANDLE_SHEBANG_SPECIALLY
    std::string& TrackingStream::get_shebang()
    {
        return shebang;
    }
#endif

    Position TrackingStream::position()
    {
        return {filename, line, col, text};
    }

    void TrackingStream::on_eof(std::string msg)
    {
        eof_message = msg;
    }

    void TrackingStream::off_eof()
    {
        eof_message.clear();
    }

    TrackingStream::operator bool()
    {
        return !text.empty();
    }

    char TrackingStream::operator *()
    {
        return text[col];
    }

    TrackingStream& TrackingStream::operator ++()
    {
        if (++col == text.length())
            next_line();
        return *this;
    }

    // for *it++, since noncopyable
    FakeTrackingStream TrackingStream::operator ++(int)
    {
        FakeTrackingStream out = *(*this);
        ++(*this);
        return out;
    }
} // namespace sexpr
} // namespace tmwa
