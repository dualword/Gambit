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

#include "SettingsContainer.hh"
#include "RegexFormatter.hh"
#include "Core/enforce.hh"
#include <boost/regex.hpp>
#include <cassert>

SettingsContainer::SettingsContainer()
{
}

SettingsContainer::SettingsContainer(const std::string &_str)
    : SettingsContainerMixin(_str)
{
}

std::string SettingsContainer::get(const std::string &key, const std::string &defaultValue) const
{
    enforce(key.length());

    boost::regex pattern("^\\h*" + key + "\\h*=\\h*(.*?)$");
    boost::smatch matches;
    if (!boost::regex_search(str_, matches, pattern))
        return defaultValue;

    return matches[1];
}

bool SettingsContainer::remove(const std::string &key)
{
    enforce(key.length());

    bool somethingWasReplaced;
    RegexReplacement r;
    r.add("");
    RegexFormatter fmt(r, &somethingWasReplaced);

    boost::regex pattern("^\\h*" + key + "\\h*=\\h*.*?(?:\r\n|\r|\n|$)");
    str_ = boost::regex_replace(str_, pattern, fmt);

    return somethingWasReplaced != 0;
}

void SettingsContainer::set(const std::string &key, const std::string &value)
{
    enforce(key.length());

    bool somethingWasReplaced;
    RegexReplacement r;
    r.add(1);
    r.add(value);
    RegexFormatter fmt(r, &somethingWasReplaced);

    boost::regex pattern("(^\\h*" + key + "\\h*=\\h*).*?$");
    boost::match_flag_type flags = boost::match_default | boost::format_first_only;
    std::string newStr = boost::regex_replace(str_, pattern, fmt, flags);
    if (!somethingWasReplaced)
    {
        // Not found, so append instead.

        size_t l = newStr.length();
        if (l != 0)
        {
            char lastChar = newStr.at(l-1);
            if (lastChar != '\r' && lastChar != '\n')
                newStr += "\n";
        }

        newStr += this->makeKeyValuePair(key, value);
    }

    str_ = newStr;
}
