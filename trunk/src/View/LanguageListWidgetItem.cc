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

#include "LanguageListWidgetItem.hh"

LanguageListWidgetItem::LanguageListWidgetItem(QLocale::Language _language)
    : language_(_language)
{
}

LanguageListWidgetItem::LanguageListWidgetItem(QLocale::Language _language, const QString &_text)
    : QListWidgetItem(_text), language_(_language)
{
}

LanguageListWidgetItem::LanguageListWidgetItem(QLocale::Language _language, const QIcon &_icon, const QString &_text)
    : QListWidgetItem(_icon, _text), language_(_language)
{
}

LanguageListWidgetItem::LanguageListWidgetItem(const LanguageListWidgetItem &other)
    : QListWidgetItem(other)
{
}

QLocale::Language LanguageListWidgetItem::language() const
{
    return language_;
}
