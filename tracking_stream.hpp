#ifndef TMWA_SEXPR_TRACKING_STREAM_HPP
#define TMWA_SEXPR_TRACKING_STREAM_HPP
//    tracking_stream.hpp - istream wrapper that keeps track of position info
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

#include <string>
#include <sstream>
#include <exception>
#include <memory>
#include <istream>
#include <fstream>

#include "defects.hpp"

/**
 * If 1, TrackingStream detects and skips a shebang line, if present.
 *
 * If 0, you can detect and skip it by checking whether the first
 * expression returned by the parser is a token at (1, 1) and starts with #!,
 * then ignore any further expressions on line 1.
 * However, this may not work if the shebang line contains any of these:
 * backslash, double-quote, open or close parenthesis
 */
#define HANDLE_SHEBANG_SPECIALLY 1
// #define HANDLE_SHEBANG_SPECIALLY 0
namespace tmwa
{
namespace sexpr
{
    struct Position
    {
        std::string filename;
        size_t line, column;
        std::string line_contents;
    };

    class Unexpected : public std::exception
    {
        std::string message;
    public:
        Unexpected(const Position& pos, const std::string& msg);
        const char *what() const noexcept override;
    };

    class FakeTrackingStream
    {
        char datum;
    public:
        FakeTrackingStream(char c);
        char operator *();
    };

    class TrackingStream
    {
        std::unique_ptr<std::istream> in;
        std::string filename, text, eof_message;
#if HANDLE_SHEBANG_SPECIALLY
        std::string shebang;
#endif
        size_t line, col;
        void next_line();
    public:
        TrackingStream(TrackingStream&&) = default;
        TrackingStream(std::string name, std::unique_ptr<std::istream> i);
        explicit TrackingStream(std::string name);
#ifdef HANDLE_SHEBANG_SPECIALLY
        std::string& get_shebang();
#endif
        Position position();
        void on_eof(std::string msg);
        void off_eof();
        explicit operator bool();
        char operator *();
        TrackingStream& operator ++();
        // for *it++, since noncopyable
        FakeTrackingStream operator ++(int);
    };
} // namespace sexpr
} // namespace tmwa

#endif //TMWA_SEXPR_TRACKING_STREAM_HPP
