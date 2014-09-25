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

#ifndef I_SETTINGS_CONTAINER_HH
#define I_SETTINGS_CONTAINER_HH

#include "SettingsElement.hh"
#include <string>

template <class T> class SettingsIterator;

class ISettingsContainer
{
public:
    typedef SettingsIterator<SettingsElement> iterator;
    typedef SettingsIterator<const SettingsElement> const_iterator;

    virtual ~ISettingsContainer() {};

    virtual iterator begin() const = 0;
    virtual iterator end() const = 0;
    virtual const_iterator constBegin() const = 0;
    virtual const_iterator constEnd() const = 0;

    virtual operator const char *() const = 0;
    virtual operator const std::string &() const = 0;
    virtual operator std::string &() = 0;
    virtual const char *operator=(const char *s) = 0;
    virtual std::string &operator=(const std::string &s) = 0;

    virtual std::string get(const std::string &, const std::string &) const = 0;

    /*
     * Returns:
     *   true/false indicating whether the key was found and removed (if it was
     *   found, it is guaranteed to have been removed).
     */
    virtual bool remove(const std::string &) = 0;

    virtual void set(const std::string &, const std::string &) = 0;

    virtual const std::string &str() const = 0;
    virtual std::string &str() = 0;
};

#endif
