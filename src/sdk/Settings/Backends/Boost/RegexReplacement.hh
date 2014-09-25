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

#ifndef REGEX_REPLACEMENT_HH
#define REGEX_REPLACEMENT_HH

#include <string>
#include <vector>

/*
 * Contains information used to replace a regex match with.
 * The match index value '-1' represents the replacement value.
 *
 * Example usage:
 *     // This constructs a RegexReplacement which replaces the first regex
 *     // match with the replacement value concatenated to the value of
 *     // sub-expression 1.
 *     RegexFormatter fmt;
 *     RegexReplacement r;
 *     r.add(1); // First replacement part is 'sub-expression 1'.
 *     r.add(value); // Second replacement part is 'value'.
 *     fmt.addReplacement(r);
 */
class RegexReplacement
{
    friend class RegexFormatter;

private:
    typedef size_t match_indice_type;
    typedef std::vector<match_indice_type> match_indices_type;
    typedef match_indices_type::const_iterator MatchIndicesIterator;

public:
    static const match_indice_type ValueIndex;

    MatchIndicesIterator begin() const;
    MatchIndicesIterator end() const;

    void add(match_indice_type);
    void add(const std::string &);
    std::string value(match_indice_type) const;

private:
    match_indices_type indices;
    std::vector<std::string> values;
};

#endif
