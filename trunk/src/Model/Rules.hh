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

#ifndef RULES_HH
#define RULES_HH

#include "Board.hh"
#include "CastlingFlags.hh"
#include "Coord.hh"
#include "EnPassant.hh"
#include "MoveHistory.hh"
#include "Piece.hh"
#include "Ply.hh"
#include "Side.hh"
#include <vector>

class PseudoMove;
class Result;
class Square;

class Rules
{
public:
    enum ReasonInvalid
    {
        ReasonInvalid_General = -1,
        ReasonInvalid_LeavesKingInCheck,
        ReasonInvalid_PutsKingInCheck,
        ReasonInvalid_Castling_KingHasMoved,
        ReasonInvalid_Castling_KingInCheck,
        ReasonInvalid_Castling_KingPassesThroughCheck,
        ReasonInvalid_Castling_RelevantRookHasMoved
    };

    Rules();
    bool canMove(const Coord &, const Coord &, Ply * = 0, ReasonInvalid * = 0) const;
    bool canUndoMove() const;
    bool canUndoPly() const;
    bool isKingInCheck(Side::Type) const;
    bool isPromotion(const Coord &, const Coord &) const;
    bool move(const Coord &, const Coord &, Piece::Type, Ply &, Result &, bool, ReasonInvalid * = 0);
    bool parseMove(Coord &, Coord &, const char *, bool &, Piece::Type &) const;
    void reset();
    void switchTurn();
    Side::Type turnParty() const;
    void undoPly();
    Side::Type opposingParty() const;

    Board board;

private:
    void __move(std::vector<std::vector<Piece> > &, const Coord &, const Coord &, const CaptureInfo &, Piece::Type) const;
    bool canMakeAnyMove(const Coord &) const;
    bool canPathBeBlocked(const std::vector<std::vector<Piece> > &, const Coord &, const Coord &, const Coord &, Side::Type) const;
    bool canSquareBeOccupied(const std::vector<std::vector<Piece> > &, const Coord &, Side::Type) const;
    void capturePiece(std::vector<std::vector<Piece> > &, const Coord &);
    void checkDraws(Result &) const;
    int getMovementDirection(const Coord &) const;
    bool isCastleMove(const Piece &, const Coord &, const Coord &) const;
    bool isCheckmate() const;
    bool isDrawByInsufficientMaterial() const;
    bool isDrawByStalemate() const;
    bool isKingInCheck(const std::vector<std::vector<Piece> > &, Side::Type) const;
    bool isPathBlocked(const std::vector<std::vector<Piece> > &, const Coord &, const Coord &, const Coord &) const;
    bool isPseudoLegalMove(const Piece &, const Coord &, const Coord &, PseudoMove &) const;
    bool isSquareAttacked(const std::vector<std::vector<Piece> > &, const Coord &, Side::Type) const;
    void needDisambiguation(const std::vector<std::vector<Piece> > &, const Coord &, const Coord &, bool &, bool &) const;

    Side::Type turn;

    int       castlingFlags;
    bool      enPassantOpportunity;
    EnPassant enPassant;

    MoveHistory history;
};

#endif
