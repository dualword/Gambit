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
#include "Core/enforce.hh"
#include "Utils/Qt.hh"
#include <QRegExp>
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

    QRegExp regex(
        QString::fromUtf8(
            ("(?:^|\r\n|\r|\n)" "\\s*" + key + "\\s*=\\s*([^\r\n]*)" "(?:\r\n|\r|\n|$)").c_str()),
        Qt::CaseSensitive, QRegExp::RegExp2);
    regex.setMinimal(true);

    QString s = QString::fromUtf8(str_.c_str());
    if (regex.indexIn(s) == -1)
        return defaultValue;

    return regex.cap(1).toUtf8().constData();
}

bool SettingsContainer::remove(const std::string &key)
{
    enforce(key.length());

    QRegExp regex(
        QString::fromUtf8(
            ("(?:^|\r\n|\r|\n)" "\\s*" + key + "\\s*=\\s*.*" "(?:\r\n|\r|\n|$)").c_str()),
        Qt::CaseSensitive, QRegExp::RegExp2);
    regex.setMinimal(true);

    QString s = QString::fromUtf8(str_.c_str());
    int pos = regex.indexIn(s);
    if (pos == -1)
        return false;

    int oldPos = pos;

    pos = Utils::Qt::QString_find_first_not_of(s, "\r\n", pos);
    assert(pos >= 0);

    QString result;
    if (pos != 0)
        result += s.mid(0, pos);
    result += s.mid(pos + (regex.matchedLength() - (pos - oldPos)));

    str_ = result.toUtf8().constData();

    return true;
}

void SettingsContainer::set(const std::string &key, const std::string &value)
{
    enforce(key.length());

    QRegExp regex(
        QString::fromUtf8(
            ("((?:^|\r\n|\r|\n)" "\\s*" + key + "\\s*=\\s*)[^\r\n]*" "(\r\n|\r|\n|$)").c_str()),
        Qt::CaseSensitive, QRegExp::RegExp2);
    regex.setMinimal(true);

    QString s = QString::fromUtf8(str_.c_str());
    int pos = regex.indexIn(s);
    if (pos == -1)
    {
        // Not found, so append instead.

        int l = s.length();
        if (l != 0)
        {
            QChar lastChar = s[l-1];
            if (lastChar != '\r' && lastChar != '\n')
                s += "\n";
        }

        str_ = s.toUtf8().constData() + this->makeKeyValuePair(key, value);
    }
    else
    {
        // NOTE: Only the first occurrence is replaced (by design).

        QString newStr = s.mid(0, pos);
        newStr += regex.cap(1);
        newStr += value.c_str() + regex.cap(2);
        newStr += s.mid(pos + regex.matchedLength());
        str_ = newStr.toUtf8().constData();
    }
}
