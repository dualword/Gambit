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

#ifndef MOVE_HISTORY_HH
#define MOVE_HISTORY_HH

#include <cstddef>
#include <vector>

class Ply;

class MoveHistory
{
public:
    MoveHistory();

    void addPly(const Ply &);
    void clear();
    const Ply &currentPly() const;
    bool canUndoMove() const;
    bool canUndoPly() const;
    const std::vector<Ply> &plies() const;
    size_t plyCount() const;
    Ply undoPly();

private:
    std::vector<Ply> plies_;
};

#endif
