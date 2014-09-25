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

#ifndef REGEX_FORMATTER_HH
#define REGEX_FORMATTER_HH

#include "RegexReplacement.hh"
#include <boost/regex.hpp>

class RegexFormatter
{
public:
    RegexFormatter(const RegexReplacement &, bool * = false);
    std::string operator()(const boost::smatch &);

private:
    std::string performReplacement(const boost::smatch &);

    const RegexReplacement &replacement;

    bool *somethingWasReplaced;
};

#endif
