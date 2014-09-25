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

#ifndef SETTINGS_ELEMENT_HH
#define SETTINGS_ELEMENT_HH

#include <string>

class SettingsElement
{
public:
    SettingsElement();
    SettingsElement(const std::string &, const std::string &);

    std::string key() const;
    std::string value() const;

private:
    std::string key_;
    std::string value_;
};

#endif
