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

#ifndef I_BOARD_VIEW_HH
#define I_BOARD_VIEW_HH

#include "Core/IEventListener.hh"
#include "Model/Coord.hh"

class BoardStyle;
class QString;
struct IBoardViewInputListener;

class IBoardView : public IEventListener
{
public:
    virtual ~IBoardView() {};

    virtual int displayWidth() const = 0;
    virtual int displayHeight() const = 0;
    virtual bool isBoardUsable() const = 0;
    virtual void cancelDragAndInvalidate() = 0;
    virtual bool isPieceSelected() const = 0;
    virtual void selectPiece(const Coord &) = 0;
    virtual Coord selectionSource() const = 0;
    virtual bool hasSelectionSource() const = 0;
    virtual void invalidateSelectionSource() = 0;
    virtual void resetSelectionInfo() = 0;
    virtual void notify(const QString &) = 0;
    virtual void notifyHide() = 0;
    virtual void notifyShow() = 0;
    virtual bool rotation() const = 0;
    virtual void setRotation(bool) = 0;
    virtual void setStyle(const BoardStyle &) = 0;
    virtual void setStyleDontSave(const BoardStyle &) = 0;
    virtual void setInputListener(IBoardViewInputListener &) = 0;
    virtual int squareSize() const = 0;
    virtual void update() = 0;
};

#endif
