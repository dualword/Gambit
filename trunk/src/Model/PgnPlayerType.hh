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

#ifndef PGN_PLAYER_TYPE_HH
#define PGN_PLAYER_TYPE_HH

#include <string>

class PgnPlayerType
{
public:
    enum Value
    {
        None,
        Human,
        Program
    };

    PgnPlayerType();
    PgnPlayerType(const std::string &s);

    bool operator==(const PgnPlayerType &other) const;
    bool operator==(const Value &value) const;

    Value get() const;

    std::string toString() const;

private:
    Value value_;
};

#endif
