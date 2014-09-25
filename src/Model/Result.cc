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

#include "Result.hh"
#include <cassert>

Result::Result()
{
    reset();
}

void Result::reset()
{
    winner = Side::None;
    isCheckmate = false;
    draw = NoDraw;
    gameEndedByResignation = false;
}

bool Result::hasResult() const
{
    if (isCheckmate || gameEndedByResignation)
        assert(winner != Side::None);
    return (winner != Side::None) || (draw != NoDraw);
}
