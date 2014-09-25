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

#ifndef LANGUAGE_LIST_WIDGET_ITEM_HH
#define LANGUAGE_LIST_WIDGET_ITEM_HH

#include <QListWidgetItem>

class LanguageListWidgetItem : public QListWidgetItem
{
public:
    LanguageListWidgetItem(QLocale::Language);
    LanguageListWidgetItem(QLocale::Language, const QString &);
    LanguageListWidgetItem(QLocale::Language, const QIcon &, const QString &);
    LanguageListWidgetItem(const LanguageListWidgetItem &);

    QLocale::Language language() const;

private:
    QLocale::Language language_;
};

#endif
