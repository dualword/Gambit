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

#ifndef SETTINGS_ITERATOR_HH
#define SETTINGS_ITERATOR_HH

#include "Core/Settings/SettingsIteratorMixin.hh"
#include <boost/regex.hpp>

template <class T>
class SettingsIterator : public SettingsIteratorMixin<T>
{
public:
    SettingsIterator(const ISettingsContainer &, size_t, size_t = 1);

protected:
    void update();
};

template <class T>
SettingsIterator<T>::SettingsIterator(const ISettingsContainer &_settingsContainer, size_t _pos, size_t _lineNumber)
    : SettingsIteratorMixin<T>(_settingsContainer, _pos, _lineNumber)
{
    update();
}

template <class T>
void SettingsIterator<T>::update()
{
    if (this->pos == std::string::npos)
        return;

    const std::string &settings = this->settingsContainer.str();
    size_t nextLinePos = findNextLinePos(this->pos, settings);
    std::string line = settings.substr(this->pos,
            (nextLinePos == std::string::npos ? std::string::npos : nextLinePos - this->pos));

    boost::regex pattern("^\\h*([a-zA-Z_]{1}(?:[a-zA-Z0-9_.][a-zA-Z0-9_]*)*)\\h*=\\h*(.*?)$");
    boost::smatch matches;
    if (!boost::regex_search(line, matches, pattern))
    {
#ifdef DEBUG
        printf("SettingsIterator: syntax error on line %lu (line = [%s]).\n", (unsigned long)this->lineNumber, line.c_str());
#endif /* defined(DEBUG) */

        this->settingsElement = SettingsElement();

        // Skip to next line or end so iterator has a valid element.
        this->pos = nextLinePos;
        update();
    }
    else
    {
        this->settingsElement = SettingsElement(matches[1], matches[2]);
    }
}

#endif
