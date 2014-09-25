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

#include "SpriteManager.hh"
#include <QPixmap>

SpriteManager::~SpriteManager()
{
    std::map<Piece::Type, std::map<Side::Type, QPixmap *> >::const_iterator piecesIt;
    std::map<Piece::Type, std::map<Side::Type, QPixmap *> >::const_iterator piecesEnd;

    piecesIt = pieceSprites.begin();
    piecesEnd = pieceSprites.end();
    for ( ; piecesIt != piecesEnd; ++piecesIt)
    {
        const std::map<Side::Type, QPixmap *> &m = piecesIt->second;
        std::map<Side::Type, QPixmap *>::const_iterator it = m.begin();
        std::map<Side::Type, QPixmap *>::const_iterator end = m.end();

        for ( ; it != end; ++it)
            delete it->second;
    }
}

void SpriteManager::add(SpriteID id, const QString &fileName)
{
    deleteSpriteIfNotNull(sprites[id]);
    sprites[id] = new QPixmap(fileName);
}

const QPixmap &SpriteManager::get(SpriteID id)
{
    return *sprites[id];
}

void SpriteManager::addPiece(Piece::Type p, Side::Type s, const QString &fileName)
{
    deleteSpriteIfNotNull(pieceSprites[p][s]);
    pieceSprites[p][s] = new QPixmap(fileName);
}

const QPixmap &SpriteManager::getPiece(Piece::Type p, Side::Type s)
{
    return *pieceSprites[p][s];
}

void SpriteManager::deleteSpriteIfNotNull(QPixmap *s) const
{
    if (s != NULL)
        delete s;
}
