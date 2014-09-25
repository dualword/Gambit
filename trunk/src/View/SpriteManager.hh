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

#ifndef SPRITE_MANAGER_HH
#define SPRITE_MANAGER_HH

#include "Model/Piece.hh"
#include "Model/Side.hh"
#include <QString>
#include <map>

class QPixmap;

enum SpriteID
{
    SPID_SEL_RECT // Selection rectangle.
};

class SpriteManager
{
public:
    ~SpriteManager();

    void add(SpriteID, const QString &);
    const QPixmap &get(SpriteID);

    void addPiece(Piece::Type, Side::Type, const QString &);
    const QPixmap &getPiece(Piece::Type, Side::Type);

private:
    void deleteSpriteIfNotNull(QPixmap *) const;

    std::map<Piece::Type, std::map<Side::Type, QPixmap *> > pieceSprites;
    std::map<SpriteID, QPixmap *> sprites;
};

#endif
