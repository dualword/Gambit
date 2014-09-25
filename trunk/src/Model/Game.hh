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

#ifndef GAME_HH
#define GAME_HH

#include "Rules.hh"
#include "Board.hh"
#include "Coord.hh"
#include "PgnMoveList.hh"
#include "PgnPlayerType.hh"
#include "Piece.hh"
#include "Result.hh"
#include "Side.hh"
#include <QString>

class IGameListener;
class Result;
class Square;

class Game
{
public:
    Game();
    void reset(Side::Type = Side::None);
    const Board &board() const;
    bool canUndoMove() const;
    bool canUndoPly() const;
    const Piece &getPiece(int, int) const;
    Side::Type humanParty() const;
    bool isInProgress() const;
    bool isMutated() const;
    void setMutation(bool);
    bool isKingInCheck(Side::Type) const;
    bool isTurnPartyPiece(const Coord &) const;
    bool isOppositePartyPiece(const Coord &, Side::Type) const;
    bool isPieceAt(int, int) const;
    bool isPromotion(const Coord &, const Coord &) const;
    const PgnMoveList &moves() const;
    Result result() const;
    void setResult(const Result &);
    void setListener(IGameListener &);
    void setSideIsEngine(Side::Type, bool);
    std::string toPGN() const;

    // Board wrappers.
    bool isWithinBounds(const Coord &) const;

    // Rules wrappers.
    bool canMove(const Coord &, const Coord &) const;
    bool move(const Coord &, const Coord &, Piece::Type, Rules::ReasonInvalid * = 0);
    Side::Type opposingParty() const;
    bool parseMove(Coord &, Coord &, const char *, bool &, Piece::Type &) const;
    Side::Type turnParty() const;
    void undoPly();

    // PGN data.
    QString whiteName;
    QString blackName;
    PgnPlayerType whiteType;
    PgnPlayerType blackType;
    QString event;

private:
    Rules rules;
    PgnMoveList moves_;
    bool isMutated_;
    Result result_;
    IGameListener *listener;
    bool whiteIsEngine,
         blackIsEngine;
};

#endif
