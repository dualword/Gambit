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

#ifndef SETTINGS_CONTAINER_MIXIN_HH
#define SETTINGS_CONTAINER_MIXIN_HH

#include "ISettingsContainer.hh"
#include "Backends/SettingsIterator.hh"

class SettingsContainerMixin : public ISettingsContainer
{
public:
    SettingsContainerMixin();
    SettingsContainerMixin(const std::string &);

    typedef SettingsIterator<SettingsElement> iterator;
    typedef SettingsIterator<const SettingsElement> const_iterator;

    iterator begin() const;
    iterator end() const;
    const_iterator constBegin() const;
    const_iterator constEnd() const;

    operator const char *() const;
    operator const std::string &() const;
    operator std::string &();
    const char *operator=(const char *);
    std::string &operator=(const std::string &);

    virtual std::string get(const std::string &, const std::string &) const = 0;

    /*
     * Returns:
     *   true/false indicating whether the key was found and removed (if it was
     *   found, it is guaranteed to have been removed).
     */
    virtual bool remove(const std::string &) = 0;

    virtual void set(const std::string &, const std::string &) = 0;

    const std::string &str() const;
    std::string &str();

protected:
    std::string makeKeyValuePair(const std::string &, const std::string &) const;

    std::string str_;
};

#endif
