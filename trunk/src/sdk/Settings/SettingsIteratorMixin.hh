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

#ifndef SETTINGS_ITERATOR_MIXIN_HH
#define SETTINGS_ITERATOR_MIXIN_HH

#include "ISettingsContainer.hh"
#include <iterator>
#include <cstdio>

template <class T>
class SettingsIteratorMixin : public std::iterator<std::forward_iterator_tag, T>
{
    // These will be const-qualified if T has a const-qualifier.
    typedef typename SettingsIteratorMixin::pointer   pointer;
    typedef typename SettingsIteratorMixin::reference reference;

public:
    SettingsIteratorMixin(const ISettingsContainer &, size_t, size_t = 1);

    reference operator*();
    pointer operator->();
    SettingsIteratorMixin &operator++();
    bool operator==(const SettingsIteratorMixin &) const;
    bool operator!=(const SettingsIteratorMixin &) const;

    size_t findNextLinePos(size_t, const std::string &) const;

protected:
    virtual void update() = 0;

    const ISettingsContainer &settingsContainer;
    SettingsElement settingsElement;
    size_t pos;
    size_t lineNumber;
};

template <class T>
SettingsIteratorMixin<T>::SettingsIteratorMixin(const ISettingsContainer &_settingsContainer, size_t _pos, size_t _lineNumber)
    : settingsContainer(_settingsContainer), pos(_pos), lineNumber(_lineNumber)
{
}

template <class T>
typename SettingsIteratorMixin<T>::reference SettingsIteratorMixin<T>::operator*()
{
    return settingsElement;
}

template <class T>
typename SettingsIteratorMixin<T>::pointer SettingsIteratorMixin<T>::operator->()
{
    return &settingsElement;
}

template <class T>
SettingsIteratorMixin<T> &SettingsIteratorMixin<T>::operator++()
{
    const std::string &settings = settingsContainer.str();
    pos = findNextLinePos(pos, settings);
    if (pos != std::string::npos)
        ++lineNumber;
    update();
    return *this;
}

template <class T>
bool SettingsIteratorMixin<T>::operator==(const SettingsIteratorMixin &rhs) const
{
    return pos == rhs.pos;
}

template <class T>
bool SettingsIteratorMixin<T>::operator!=(const SettingsIteratorMixin &rhs) const
{
    return !(*this == rhs);
}

template <class T>
size_t SettingsIteratorMixin<T>::findNextLinePos(size_t _pos, const std::string &str) const
{
    if (_pos == std::string::npos)
        return std::string::npos;

    size_t nextLinePos = str.find_first_of("\r\n", _pos);
    if (nextLinePos == std::string::npos)
        return std::string::npos;

    // Since we only matched one character, we need to check whether we matched
    // a "\r\n" sequence, or just a single newline character.
    if (str[nextLinePos] == '\r' && str[nextLinePos+1] == '\n')
        ++nextLinePos;

    ++nextLinePos; // Move to the first character after the EOL sequence.

    if (str[nextLinePos] == '\0')
        nextLinePos = std::string::npos;
    return nextLinePos;
}

#endif
