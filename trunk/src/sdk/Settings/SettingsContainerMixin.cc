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

#include "SettingsContainerMixin.hh"

SettingsContainerMixin::SettingsContainerMixin()
{
}

SettingsContainerMixin::SettingsContainerMixin(const std::string &_str)
    : str_(_str)
{
}

SettingsContainerMixin::iterator SettingsContainerMixin::begin() const
{
    return SettingsContainerMixin::iterator(*this, 0);
}

SettingsContainerMixin::iterator SettingsContainerMixin::end() const
{
    return SettingsContainerMixin::iterator(*this, std::string::npos);
}

SettingsContainerMixin::const_iterator SettingsContainerMixin::constBegin() const
{
    return SettingsContainerMixin::const_iterator(*this, 0);
}

SettingsContainerMixin::const_iterator SettingsContainerMixin::constEnd() const
{
    return SettingsContainerMixin::const_iterator(*this, std::string::npos);
}

SettingsContainerMixin::operator const char *() const
{
    return str_.c_str();
}

SettingsContainerMixin::operator const std::string &() const
{
    return str_;
}

SettingsContainerMixin::operator std::string &()
{
    return str_;
}

const char *SettingsContainerMixin::operator=(const char *s)
{
    str_ = s;
    return str_.c_str();
}

std::string &SettingsContainerMixin::operator=(const std::string &s)
{
    str_ = s;
    return str_;
}

const std::string &SettingsContainerMixin::str() const
{
    return str_;
}

std::string &SettingsContainerMixin::str()
{
    return str_;
}

std::string SettingsContainerMixin::makeKeyValuePair(const std::string &key, const std::string &value) const
{
    return key + " = " + value + "\n";
}
