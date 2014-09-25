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

#include "../../SettingsIteratorMixin.hh"
#include <QRegExp>

template <typename T>
class SettingsIterator : public SettingsIteratorMixin<T>
{
public:
    SettingsIterator(const ISettingsContainer &, size_t, size_t = 1);

protected:
    void update();
};

template <typename T>
SettingsIterator<T>::SettingsIterator(const ISettingsContainer &_settingsContainer, size_t _pos, size_t _lineNumber)
    : SettingsIteratorMixin<T>(_settingsContainer, _pos, _lineNumber)
{
    update();
}

template <typename T>
void SettingsIterator<T>::update()
{
    if (this->pos == std::string::npos)
        return;

    const std::string &settings = this->settingsContainer.str();
    size_t nextLinePos = this->findNextLinePos(this->pos, settings);
    QString line = QString::fromUtf8(
        settings.substr(
            this->pos,
            (nextLinePos == std::string::npos ? std::string::npos : nextLinePos - this->pos)).c_str());

    QRegExp regex(
        QString::fromUtf8(
            "^\\s*([a-zA-Z_]{1}(?:[a-zA-Z0-9_.][a-zA-Z0-9_]*)*)\\s*=\\s*([^\r\n]*)" "(?:\r\n|\r|\n|$)"),
        Qt::CaseSensitive, QRegExp::RegExp2);
    regex.setMinimal(true);

    if (regex.indexIn(line) == -1)
    {
#ifdef DEBUG
        printf("SettingsIterator: syntax error on line %lu (line = [%s]).\n", (unsigned long)this->lineNumber, qPrintable(line));
#endif /* defined(DEBUG) */

        this->settingsElement = SettingsElement();

        // Skip to next line or end so iterator has a valid element.
        this->pos = nextLinePos;
        update();
    }
    else
    {
        this->settingsElement = SettingsElement(
            regex.cap(1).toUtf8().constData(),
            regex.cap(2).toUtf8().constData());
    }
}

#endif
