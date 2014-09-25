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

#include "RegexFormatter.hh"

RegexFormatter::RegexFormatter(const RegexReplacement &_replacement,
        bool *_somethingWasReplaced /* = false */)
    : replacement(_replacement), somethingWasReplaced(_somethingWasReplaced)
{
    if (somethingWasReplaced)
        *somethingWasReplaced = false;
}

std::string RegexFormatter::operator()(const boost::smatch &what)
{
    return performReplacement(what);
}

std::string RegexFormatter::performReplacement(const boost::smatch &matches)
{
    std::string s;

    RegexReplacement::MatchIndicesIterator it = replacement.begin();
    for ( ; it != replacement.end(); ++it)
    {
        if (*it & RegexReplacement::ValueIndex)
            s += replacement.value(*it & ~RegexReplacement::ValueIndex);
        else
        {
            assert(matches[*it].matched);

            // Use the numbered capture as replacement.
            s += matches[*it];
        }
    }

    if (somethingWasReplaced)
        *somethingWasReplaced = true;

    return s;
}
