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

#ifndef LANGUAGE_LIST_WIDGET_HH
#define LANGUAGE_LIST_WIDGET_HH

#include <QListWidget>

class LanguageListWidgetItem;

class LanguageListWidget : public QListWidget
{
public:
    LanguageListWidget(QWidget * = 0);
    void addItem(const QString &);
    void addItem(LanguageListWidgetItem *);
    void emphasizeSelectedLanguage();
    int findLanguageRow(QLocale::Language) const;
    QLocale::Language selectedLanguage() const;
    void setCurrentItem(QListWidgetItem *);
    void setCurrentItem(QListWidgetItem *, QItemSelectionModel::SelectionFlags);
    void setCurrentRow(int);
    void setCurrentRow(int, QItemSelectionModel::SelectionFlags);

private:
    QListWidgetItem *emphasizedItem;
};

#endif
