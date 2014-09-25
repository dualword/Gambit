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

#include "RegexReplacement.hh"
#include <cassert>
#include <cstdio>

const RegexReplacement::match_indice_type RegexReplacement::ValueIndex =
    (size_t)1 << (sizeof(match_indice_type) * 8 - 1);

RegexReplacement::MatchIndicesIterator RegexReplacement::begin() const
{
    return indices.begin();
}

RegexReplacement::MatchIndicesIterator RegexReplacement::end() const
{
    return indices.end();
}

void RegexReplacement::add(match_indice_type i)
{
    indices.push_back(i);
}

void RegexReplacement::add(const std::string &s)
{
    values.push_back(s);
    indices.push_back(ValueIndex | (values.size() - 1));
}

std::string RegexReplacement::value(match_indice_type index) const
{
    assert(index < values.size());
    return values[index];
}
