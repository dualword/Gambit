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

#ifndef SETTINGS_CONTAINER_HH
#define SETTINGS_CONTAINER_HH

#include "../../SettingsContainerMixin.hh"

class SettingsContainer : public SettingsContainerMixin
{
public:
    SettingsContainer();
    SettingsContainer(const std::string &);

    std::string get(const std::string &, const std::string &) const;
    bool remove(const std::string &);
    void set(const std::string &, const std::string &);
};

#endif
