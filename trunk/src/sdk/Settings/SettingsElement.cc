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

#include "SettingsElement.hh"

SettingsElement::SettingsElement()
{
}

SettingsElement::SettingsElement(const std::string &_key, const std::string &_value)
    : key_(_key), value_(_value)
{
}

std::string SettingsElement::key() const
{
    return key_;
}

std::string SettingsElement::value() const
{
    return value_;
}
