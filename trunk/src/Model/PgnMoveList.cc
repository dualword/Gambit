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

#include "PgnMoveList.hh"
#include <cstdio>
#include <cstring>
#include <cassert>

void PgnMoveList::addPly(const Ply &ply)
{
    plies_.push_back(ply);
}

void PgnMoveList::clear()
{
    plies_.clear();
}

const std::vector<Ply> &PgnMoveList::plies() const
{
    return plies_;
}

size_t PgnMoveList::plyCount() const
{
    return plies_.size();
}

void PgnMoveList::undoPly()
{
    assert(plies_.size() != 0);
    plies_.pop_back();
}

static void appendMoveText(std::string &buf, const char *s, int &linelen)
{
    size_t i = 0, len;

    len = strlen(s);
    if (len == 0)
        return;

    linelen += len;
    if (linelen > 71)
    {
        buf += '\n';

        // Skip leading whitespace in string when starting on a new line so it
        // won't be appended.
        for ( ; s[i] == ' '; ++i)
            ;

        linelen = strlen(&s[i]);
    }

    if (s[i] != '\0')
        buf += &s[i];
}

std::string PgnMoveList::toString() const
{
    std::string s;
    char buf[16];
    std::vector<Ply>::const_iterator it = plies_.begin();
    std::vector<Ply>::const_iterator end = plies_.end();
    int i = 0, j = 1, linelen = 0;

    for ( ; it != end; ++i, ++it)
    {
        int r;

        if (!(i & 1))
        {
            r = snprintf(buf, sizeof buf, "%s%d.", j == 1 ? "" : " ", j);
            if (r < 0 || (unsigned)r >= sizeof buf)
                assert(0 && "Buffer size inadequate");
            appendMoveText(s, buf, linelen);

            ++j;
        }

        r = snprintf(buf, sizeof buf, " %s", it->toString().c_str());
        if (r < 0 || (unsigned)r >= sizeof buf)
            assert(0 && "Buffer size inadequate");
        appendMoveText(s, buf, linelen);
    }

    return s;
}
