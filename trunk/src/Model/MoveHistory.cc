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

#include "MoveHistory.hh"
#include "Ply.hh"
#include <cassert>

MoveHistory::MoveHistory()
{
    clear();
}

void MoveHistory::addPly(const Ply &ply)
{
    plies_.push_back(ply);
}

void MoveHistory::clear()
{
    plies_.clear();
}

const Ply &MoveHistory::currentPly() const
{
    return plies_.at(plyCount() - 1);
}

bool MoveHistory::canUndoMove() const
{
    return plyCount() >= 2;
}

bool MoveHistory::canUndoPly() const
{
    return plyCount() >= 1;
}

const std::vector<Ply> &MoveHistory::plies() const
{
    return plies_;
}

size_t MoveHistory::plyCount() const
{
    return plies_.size();
}

Ply MoveHistory::undoPly()
{
    assert(canUndoPly());

    Ply ply = plies_.at(plyCount() - 1);
    plies_.resize(plyCount() - 1);
    return ply;
}
