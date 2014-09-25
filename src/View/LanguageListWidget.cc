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

#include "LanguageListWidget.hh"
#include "LanguageListWidgetItem.hh"
#include "Utils/Cast.hh"
#include <cassert>
#include <typeinfo>
using Utils::Cast::enforce_dynamic_cast;

LanguageListWidget::LanguageListWidget(QWidget *_parent /* = 0 */)
    : QListWidget(_parent), emphasizedItem(0)
{
}

void LanguageListWidget::addItem(const QString &)
{
    // Do not use this function. Use addItem(LanguageListWidgetItem *) instead
    // so that we can be sure every item is a LanguageListWidgetItem which is
    // bound to a language.
    assert(0);
}

void LanguageListWidget::addItem(LanguageListWidgetItem *_item)
{
    QListWidget::addItem(_item);
}

void LanguageListWidget::emphasizeSelectedLanguage()
{
    if (emphasizedItem)
    {
        QFont _font(emphasizedItem->font());
        _font.setWeight(QFont::Normal);
        emphasizedItem->setFont(_font);
        emphasizedItem = 0;
    }

    QListWidgetItem *_item = currentItem();
    if (_item)
    {
        QFont _font(_item->font());
        _font.setWeight(QFont::Bold);
        _item->setFont(_font);
        emphasizedItem = _item;
    }
}

int LanguageListWidget::findLanguageRow(QLocale::Language language) const
{
    (void)language;

    for (int i = 0; i < count(); ++i)
    {
        QLocale::Language l;

        // We assume all items in the list are of type LanguageListWidgetItem.
        l = enforce_dynamic_cast<LanguageListWidgetItem *>(QListWidget::item(i))->language();

        if (l == language)
            return i;
    }

    return -1;
}

QLocale::Language LanguageListWidget::selectedLanguage() const
{
    // We assume all items in the list are of type LanguageListWidgetItem.
    return enforce_dynamic_cast<LanguageListWidgetItem *>(QListWidget::currentItem())->language();
}

void LanguageListWidget::setCurrentItem(QListWidgetItem *_item)
{
    QListWidget::setCurrentItem(_item);

    emphasizeSelectedLanguage();
}

void LanguageListWidget::setCurrentItem(QListWidgetItem *_item, QItemSelectionModel::SelectionFlags command)
{
    QListWidget::setCurrentItem(_item, command);

    emphasizeSelectedLanguage();
}

void LanguageListWidget::setCurrentRow(int _row)
{
    QListWidget::setCurrentRow(_row);

    emphasizeSelectedLanguage();
}

void LanguageListWidget::setCurrentRow(int _row, QItemSelectionModel::SelectionFlags command)
{
    QListWidget::setCurrentRow(_row, command);

    emphasizeSelectedLanguage();
}
