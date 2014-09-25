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

#include "BoardStyles.hh"
#include <QtGlobal>
#include <algorithm>
#include <cassert>

int BoardStyles::defaultStyleIndex_ = -1;

BoardStyles::vector_type BoardStyles::styles_;

class BoardStyleFindPredicate
{
public:
    BoardStyleFindPredicate(const QString &name)
        : name_(name)
    {
    }

    bool operator()(const BoardStyle &style)
    {
        return style.name == name_;
    }

private:
    QString name_;
};

BoardStyle BoardStyles::get(size_t index)
{
    ensureInitialized();
    return styles_.at(index);
}

BoardStyle BoardStyles::get(const QString &name)
{
    ensureInitialized();

    vector_type::const_iterator it =
        std::find_if(
            styles_.begin(),
            styles_.end(),
            BoardStyleFindPredicate(name));

    if (it == styles_.end())
        return getDefault();

    return *it;
}

const BoardStyles::vector_type &BoardStyles::getAll()
{
    ensureInitialized();
    return styles_;
}

BoardStyle BoardStyles::getDefault()
{
    ensureInitialized();

    assert(defaultStyleIndex_ != -1);
    return styles_.at(defaultStyleIndex_);
}

void BoardStyles::ensureInitialized()
{
    if (styles_.size())
        return;

    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Blue - Dark Cerulean"), "#5D91BD", "#2C639F"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Blue - Gambit"), "#91AEC0", "#4F6D91"));
    defaultStyleIndex_ = styles_.size() - 1;
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Blue - Steel"), "#82B6DB", "#4682B4"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Blue - Teal"), "#70A9BA", "#367588"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Brown - Beaver"), "#CDAE89", "#86706C"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Brown - Chocolate"), "#B3977F", "#87705D"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Gray - Dim"), "#A9A9A9", "#696969"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Gray - Silver"), "#C0C0C0", "#888888"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Green - Asparagus"), "#FFDEAD", "#96B47E"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Green - Moss"), "#B7C0A9", "#7F916F"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Green - Olive"), "#C1C690", "#7C896C"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Orange - Morning"), "#FBCEB1", "#C8A67E"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Orange - Narcissus"), "#D9C882", "#C19A6B"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Purple - Fuchsia"), "#BCA9C8", "#A0739A"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Purple - Thistle"), "#E19DC5", "#AE628F"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Red - Maroon"), "#ED9595", "#9B5B5B"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Red - Wine"), "#D9A2A2", "#99696F"));
    styles_.push_back(BoardStyle(/*QT_TRANSLATE_NOOP("UI", */"XBoard", "#C8C365", "#77A26D"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Yellow - Buff"), "#F0DC82", "#C19A6B"));
    styles_.push_back(BoardStyle(QT_TRANSLATE_NOOP("UI", "Yellow - Saffron"), "#C8C365", "#A4825A"));
}
