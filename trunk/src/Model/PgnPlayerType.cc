/*
    Written by Jelle Geerts (jellegeerts@gmail.com).

    To the extent possible under law, the author(s) have dedicated all
    copyright and related and neighboring rights to this software to
    the public domain worldwide. This software is distributed without
    any warranty.

    You should have received a copy of the CC0 Public Domain Dedication
    along with this software.
    If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "PgnPlayerType.hh"
#include <cassert>

PgnPlayerType::PgnPlayerType()
    : value_(None)
{
}

PgnPlayerType::PgnPlayerType(const std::string &s)
    : value_(None)
{
    if (s.compare("human") == 0)
        value_ = Human;
    else if (s.compare("program") == 0)
        value_ = Program;
    else
        assert(value_ == None);
}

bool PgnPlayerType::operator==(const PgnPlayerType &other) const
{
    return value_ == other.value_;
}

bool PgnPlayerType::operator==(const Value &value) const
{
    return value_ == value;
}

PgnPlayerType::Value PgnPlayerType::get() const
{
    return value_;
}

std::string PgnPlayerType::toString() const
{
    switch (value_)
    {
    case Human:
        return "human";

    case Program:
        return "program";

    default:
        assert(value_ == None);
        // A question mark is normally used in PGN to denote something unknown.
        return "?";
    }
}
