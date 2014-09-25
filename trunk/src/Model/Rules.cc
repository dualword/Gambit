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

#include "Rules.hh"
#include "MoveNotation.hh"
#include "Result.hh"
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cctype>
using std::vector;

enum PathDirection
{
    MR_PATH_N,
    MR_PATH_NW,
    MR_PATH_W,
    MR_PATH_SW,
    MR_PATH_S,
    MR_PATH_SE,
    MR_PATH_E,
    MR_PATH_NE
};

static const Coord kingDeltas[] =
{
    Coord(-1,  0),
    Coord(-1, -1),
    Coord( 0, -1),
    Coord(+1, -1),
    Coord(+1,  0),
    Coord(+1, +1),
    Coord( 0, +1),
    Coord(-1, +1)
};

static const Coord rookDeltas[] =
{
    Coord(-1,  0),
    Coord( 0, -1),
    Coord(+1,  0),
    Coord( 0, +1)
};

static const Coord bishopDeltas[] =
{
    Coord(-1, -1),
    Coord(+1, -1),
    Coord(+1, +1),
    Coord(-1, +1)
};

static const Coord knightDeltas[] =
{
    Coord(-2, -1),
    Coord(-1, -2),
    Coord(+1, -2),
    Coord(+2, -1),
    Coord(+2, +1),
    Coord(+1, +2),
    Coord(-1, +2),
    Coord(-2, +1)
};

static const Coord *deltas[] =
{
    kingDeltas,
    kingDeltas, // Queen deltas are equal to the king deltas.
    rookDeltas,
    bishopDeltas,
    knightDeltas
};

static const size_t deltaArrayLengths[] =
{
    sizeof kingDeltas / sizeof kingDeltas[0],
    sizeof kingDeltas / sizeof kingDeltas[0],
    sizeof rookDeltas / sizeof rookDeltas[0],
    sizeof bishopDeltas / sizeof bishopDeltas[0],
    sizeof knightDeltas / sizeof knightDeltas[0]
};

static const int castlingMasksKing[] =
{
    CastlingFlags::WhiteKingIsNotAvailable,
    CastlingFlags::BlackKingIsNotAvailable
};

static const int castlingMasksRook[][2] =
{
    {
        CastlingFlags::WhiteKingsRookIsNotAvailable,
        CastlingFlags::WhiteQueensRookIsNotAvailable
    },
    {
        CastlingFlags::BlackKingsRookIsNotAvailable,
        CastlingFlags::BlackQueensRookIsNotAvailable
    }
};

class PseudoMove
{
public:
    PseudoMove()
        : isTwoStepPawnMove(false), isPawnCapture(false) {};

    bool isTwoStepPawnMove;
    bool isPawnCapture;
};

Rules::Rules()
{
    reset();
}

// This function won't check whether the move is made by the party which is to move. This is done
// for simplicity (so this function is much more reusable). It is assumed this check is done
// elsewhere.
bool Rules::canMove(const Coord &from, const Coord &to, Ply *_ply /* = 0 */, ReasonInvalid *reasonInvalid /* = 0 */) const
{
    Ply ply;

    ply.castlingFlags = castlingFlags;

    if (reasonInvalid)
        *reasonInvalid = ReasonInvalid_General;

    if (!board.isWithinBounds(from) || !board.isWithinBounds(to))
        return false;

    const Piece &piece = board.pieces[from.x][from.y];
    assert((piece.type != Piece::None) && "There should be a piece on the given location");

    Side::Type side = piece.side;

    bool kingInCheckAtStartOfTurn = isKingInCheck(board.pieces, side);

    // Movement vector (relative).
    Coord movementVector(to.x - from.x, to.y - from.y);

    // Don't allow capturing of one's own pieces.
    if ((board.pieces[to.x][to.y].type != Piece::None) && (board.pieces[to.x][to.y].side == side))
        return false;

    /*
     * Now check if the move is within the bounds of that piece's movement constraints.
     */

    assert(piece.type != Piece::None);

    PseudoMove pseudoMove;
    bool _isCastleMove = isCastleMove(piece, to, movementVector);
    if (!_isCastleMove && !isPseudoLegalMove(piece, from, movementVector, pseudoMove))
        return false;

    /*
     * Check if any other piece is blocking the way.
     */

    switch (piece.type)
    {
    case Piece::Rook:
    case Piece::Bishop:
    case Piece::Queen:
    {
        if (isPathBlocked(board.pieces, from, to, movementVector))
            return false;
        break;
    }
    case Piece::Pawn:
    {
        // For the pawn we only need to check if there is a piece blocking the path in case the
        // pawn is moving 2 steps.
        if ((abs(movementVector.y) == 2) && isPathBlocked(board.pieces, from, to, movementVector))
            return false;
        break;
    }
    default:
    {
        // No check needed for the other piece types since they can only move one delta at a time.
        break;
    }
    }

    /*
     * Check pawn movement and the En Passant move.
     */

    bool isEnPassant = false;

    // Phase 1: check if we're moving a pawn and if it might be able to do an En Passant move.
    if ((piece.type == Piece::Pawn) && enPassantOpportunity)
    {
        // Check if it's a valid En Passant move. It is only valid if the piece at the capture
        // location is a piece of the opposite side. This assumes the pawn *has* to be performing
        // an En Passant move (which is always true if we correctly reset the En Passant
        // opportunity).

        const Piece &p = board.pieces[enPassant.captureLocation.x][enPassant.captureLocation.y];
        assert(p.type != Piece::None);
        if ((to == enPassant.location) && (side != p.side))
        {
            assert(pseudoMove.isPawnCapture);
            isEnPassant = true;
        }
    }

    // If the pawn is making a 'capture move', it must be capturing a piece at its destination,
    // unless it is performing an En Passant move.
    if (pseudoMove.isPawnCapture && (board.pieces[to.x][to.y].type == Piece::None) && !isEnPassant)
        return false;

    // If the target square of the pawn move contains another piece, the pawn must be moving along
    // the X axis for this move to constitute a valid capture move. This prevents pawns from
    // capturing by moving forward.
    if ((piece.type == Piece::Pawn) && (board.pieces[to.x][to.y].type != Piece::None) && (movementVector.x == 0))
        return false;

    // If a pawn is performing the En Passant move, store the location of the piece that this move
    // would capture.
    if (pseudoMove.isPawnCapture && isEnPassant)
    {
        // Capture location of En Passant should never be 0,0.
        assert(enPassant.captureLocation.x | enPassant.captureLocation.y);

        ply.isCapture = true;
        ply.ci.location = enPassant.captureLocation;
        ply.ci.piece = board.pieces[enPassant.captureLocation.x][enPassant.captureLocation.y];
    }

    // Phase 2: check if we're moving a pawn two steps. Also, in that case, there may be
    // pieces that can (on the next move) perform the En Passant move.
    if (piece.type == Piece::Pawn && pseudoMove.isTwoStepPawnMove)
    {
        /*
         * Save the location that's available for the En Passant move.
         */

        int direction = getMovementDirection(movementVector);

        switch (direction)
        {
        case MR_PATH_N:
        {
            ply.enPassant.location = Coord(to.x, to.y + 1);
            break;
        }

        case MR_PATH_S:
        {
            ply.enPassant.location = Coord(to.x, to.y - 1);
            break;
        }

        default:
            assert(0 && "A pawn should only be able to move forward or backwards when moving two steps at once");
            break;
        }

        // Store the location of the pawn that would be captured after the En Passant move.
        ply.enPassant.captureLocation = to;

        // The next move there may be an opportunity for the opponent to do an En Passant move.
        ply.enPassantOpportunity = true;
    }

    /*
     * Perform Castling checks.
     */

    // Trying to castle?
    if (_isCastleMove)
    {
        bool castleQueenSide = from.x > to.x;

        /*
         * Phase 1:
         * The king and rook must not have moved.
         */

        if ((castlingFlags & castlingMasksKing[piece.side]) != 0)
        {
            if (reasonInvalid)
                *reasonInvalid = ReasonInvalid_Castling_KingHasMoved;
            return false;
        }
        if ((castlingFlags & castlingMasksRook[piece.side][castleQueenSide]) != 0)
        {
            // The rook relevant to the attempted castling move has moved.
            if (reasonInvalid)
                *reasonInvalid = ReasonInvalid_Castling_RelevantRookHasMoved;
            return false;
        }

        /*
         * Phase 2:
         * The king must not be in check.
         */

        if (isSquareAttacked(board.pieces, from, Side::opposite(side)))
        {
            if (reasonInvalid)
                *reasonInvalid = ReasonInvalid_Castling_KingInCheck;
            return false;
        }

        // No need to perform boundary checks, as we already verified that the king hasn't moved.
        Piece castleRook = castleQueenSide ? board.pieces[from.x - 4][from.y] : board.pieces[from.x + 3][from.y];
        Coord castleRookSource = castleQueenSide ? Coord(from.x - 4, from.y) : Coord(from.x + 3, from.y);

        // We already verified that the king and rook didn't move, so we must have a rook.
        assert(castleRook.type != Piece::None);
        (void)castleRook;

        /*
         * Phase 3:
         * No square between the king and the rook may be occupied.
         */

        if (isPathBlocked(board.pieces, from, castleRookSource, movementVector))
            return false;

        /*
         * Phase 4:
         * Check if the square the king passes through is attacked.
         * No need to check the king's destination, since that's part of
         * the rule that says the king should not be in check after a move.
         */

        // Check square next to the king (the square adjacent to that one is the square the king
        // will be put on, so that will be covered by the king-in-check check).
        if (isSquareAttacked(board.pieces, Coord(from.x + (castleQueenSide ? -1 : +1), from.y), Side::opposite(side)))
        {
            if (reasonInvalid)
                *reasonInvalid = ReasonInvalid_Castling_KingPassesThroughCheck;
            return false;
        }

        // The castle move is valid.
        ply.isCastle = true;
        ply.csi.rookSource = castleRookSource;
        ply.csi.rookDestination = castleQueenSide ? Coord(from.x - 1, from.y) : Coord(from.x + 1, from.y);
    }

    /*
     * Save the new castling flags in the ply.
     */

    switch (piece.type)
    {
    case Piece::King:
        CastlingFlags::setKingNotAvailableForCastling(ply.castlingFlags, piece.side);
        break;

    case Piece::Rook:
        if (piece.side == Side::White)
        {
            if ((from.x == board.width - 1) && (from.y == board.height - 1))
                CastlingFlags::setRookNotAvailableForCastling(ply.castlingFlags, piece.side, false);
            else if ((from.x == 0) && (from.y == board.height - 1))
                CastlingFlags::setRookNotAvailableForCastling(ply.castlingFlags, piece.side, true);
        }
        else
        {
            if ((from.x == (board.width - 1)) && (from.y == 0))
                CastlingFlags::setRookNotAvailableForCastling(ply.castlingFlags, piece.side, false);
            else if ((from.x == 0) && (from.y == 0))
                CastlingFlags::setRookNotAvailableForCastling(ply.castlingFlags, piece.side, true);
        }
        break;

    default:
        break;
    }

    /*
     * If the move would capture a piece, store the location to the captured piece.
     */

    // This automatically excludes En Passant captures (as the location of the piece captured with
    // an En Passant move is not equal to the destination of an En Passant move), which are handled
    // elsewhere.
    if (board.pieces[to.x][to.y].type != Piece::None)
    {
        ply.isCapture = true;
        ply.ci.location = to;
        ply.ci.piece = board.pieces[to.x][to.y];
        assert(ply.ci.piece.side != side);

        // Also, if the capture move captures one of the opponent's rooks, check whether it was in
        // one of the starting positions required for castling (this will also be true when one
        // captures a rook that promoted and moved back to one of the starting positions, but
        // that's okay). When such a rook is captured, it is not available anymore for castling
        // (this safeguards against allowing castling moves by promoting a rook and moving it back
        // to one of the starting positions (which necessitates that the original rook on either of
        // those starting positions either moved, which is checked elsewhere, or was captured).
        if (ply.ci.piece.type == Piece::Rook)
        {
            bool updateCastlingFlags = false,
                 queenSideRook;

            if ((ply.ci.location == Coord(0, 0)) ||
                (ply.ci.location == Coord(0, board.height - 1)))
            {
                queenSideRook = true;
                updateCastlingFlags = true;
            }
            else if ((ply.ci.location == Coord(board.width - 1, 0)) ||
                     (ply.ci.location == Coord(board.width - 1, board.height - 1)))
            {
                queenSideRook = false;
                updateCastlingFlags = true;
            }

            if (updateCastlingFlags)
            {
                CastlingFlags::setRookNotAvailableForCastling(ply.castlingFlags, ply.ci.piece.side,
                                                              queenSideRook);
            }
        }
    }

    /*
     * See if our king would be in check after the move is performed.
     */

    vector<vector<Piece> > kingCheckPieces = board.pieces; // Make a copy.
    // Promotion piece doesn't matter but must be valid nonetheless, because this function may be
    // called from canMakeAnyMove(), which may try to move a pawn to the last rank, in which case
    // __move() will assert that the promotion piece is valid. We could remove the assertion, of
    // course, but by not doing that, and simply specifying a promotion piece here, we can still
    // benefit from having the assert() in __move(), as it can catch errors made elsewhere.
    __move(kingCheckPieces, from, to, ply.ci, Piece::Queen);
    if (isKingInCheck(kingCheckPieces, side))
    {
        if (reasonInvalid)
        {
            // If the king was not in check at the start of the turn, then this move would put the
            // king in check. Otherwise, it would leave the king in check.
            if (!kingInCheckAtStartOfTurn)
                *reasonInvalid = ReasonInvalid_PutsKingInCheck;
            else
                *reasonInvalid = ReasonInvalid_LeavesKingInCheck;
        }
        return false;
    }

    ply.isEnPassant = isEnPassant;

    if (ply.isCapture)
    {
        // There should be a piece at the capture location.
        assert(board.pieces[ply.ci.location.x][ply.ci.location.y].type != Piece::None);
    }

    if (_ply)
        *_ply = ply;
    return true;
}

bool Rules::canUndoMove() const
{
    return history.canUndoMove();
}

bool Rules::canUndoPly() const
{
    return history.canUndoPly();
}

bool Rules::isKingInCheck(Side::Type side) const
{
    return isKingInCheck(board.pieces, side);
}

bool Rules::isPromotion(const Coord &from, const Coord &to) const
{
    return (board.pieces[from.x][from.y].type == Piece::Pawn) &&
           ((to.y == 0) || (to.y == 7));
}

bool Rules::move(const Coord &from, const Coord &to, Piece::Type promotion,
    Ply &ply, Result &result, bool saveHistory, ReasonInvalid *reasonInvalid /* = 0 */)
{
    assert(board.pieces[from.x][from.y].type != Piece::None && "No piece present at the given location");

    /*
     * Phase 1: performing movement rule checks.
     */

    if (!canMove(from, to, &ply, reasonInvalid))
        return false;

    /*
     * Phase 2: the move is legitimate.
     */

    if (ply.isCapture)
        capturePiece(board.pieces, ply.ci.location);

    /*
     * Move the piece to the new location.
     */

    ply.from = from;
    ply.to = to;
    ply.piece = board.pieces[from.x][from.y];
    needDisambiguation(board.pieces, from, to, ply.disambiguateFile, ply.disambiguateRank);

    if (isPromotion(from, to))
        ply.promotion = promotion;

    __move(board.pieces, from, to, ply.ci, promotion);

    ply.isCheck = isKingInCheck(board.pieces, Side::opposite(turn));

    if (ply.isCastle)
        __move(board.pieces, ply.csi.rookSource, ply.csi.rookDestination, ply.ci, Piece::None);

    enPassantOpportunity = ply.enPassantOpportunity;
    if (ply.enPassantOpportunity)
        enPassant = ply.enPassant;

    castlingFlags = ply.castlingFlags;

    ply.isCheckmate = isCheckmate();
    result.isCheckmate = ply.isCheckmate;
    if (result.isCheckmate)
        result.winner = turn;
    else
        checkDraws(result);

    if (saveHistory)
        history.addPly(ply);

    switchTurn();
    return true;
}

bool Rules::parseMove(Coord &from, Coord &to, const char *_move,
                              bool &isPawnPromotion, Piece::Type &promotion) const
{
    int         i, movelen;
    Piece::Type pieceType = Piece::None;
    int         sourceFile = -1, sourceRank = -1;
    int         rankLimit = board.height - 1;
    int         fileLimit = board.width - 1;
    bool        isSanMove = false;

    isPawnPromotion = false;

    movelen = strlen(_move);
    if (!movelen)
        return false;

    /* TODO do validations like below all in one place, not in multiple places
                if (to.x < 0 || to.x > 7)
                    return false;
                if (to.y < 0 || to.y > 7)
                    return false;
     */

    if (islower(_move[0]))
    {
        // CAN move, or SAN pawn move.
        Coord c;

        if (movelen < 2)
            return false;

        i = 0;
        if (_move[1] == 'x')
        {
            isSanMove = true;

            sourceFile = _move[0] - 'a';
            if (sourceFile < 0 || sourceFile > 7)
                return false;

            if (movelen < 4)
                return false;
            else
                i = 2;
        }

        c.x = _move[i] - 'a';
        if (c.x < 0 || c.x > 7)
            return false;
        ++i;
        c.y = 7 - (_move[i] - '1');
        if (c.y < 0 || c.y > 7)
            return false;

        if (movelen < 4)
            isSanMove = true;
        else if (movelen >= 4)
        {
            char ch;

            // Assume move is a pawn promotion, reset boolean later if not.
            isPawnPromotion = true;

            if (_move[2] == '=')
            {
                // SAN pawn promotion.
                isSanMove = true;
                ch = _move[3];
            }
            else if (movelen > 5 && _move[4] == '=')
            {
                // SAN pawn promotion.
                isSanMove = true;
                ch = _move[5];
            }
            else if (!isSanMove && movelen > 4)
            {
                // CAN pawn promotion.
                ch = toupper(_move[4]);
            }
            else
                isPawnPromotion = false;

            if (isPawnPromotion)
            {
                switch (ch)
                {
                case 'Q':
                    promotion = Piece::Queen;
                    break;
                case 'R':
                    promotion = Piece::Rook;
                    break;
                case 'B':
                    promotion = Piece::Bishop;
                    break;
                case 'N':
                    promotion = Piece::Knight;
                    break;
                default:
                {
                    // TODO
                    //Log::msg(Log::Debug, "unrecognized promotion piece type '%c'\n", c);
                    return false;
                }
                }
            }
        }

        if (isSanMove)
        {
            // SAN move.
            pieceType = Piece::Pawn;
            to = c;
        }
        else
        {
            if (movelen < 4)
                return false;

            // CAN move.
            to.x = _move[2] - 'a';
            if (to.x < 0 || to.x > 7)
                return false;
            to.y = 7 - (_move[3] - '1');
            if (to.y < 0 || to.y > 7)
                return false;

            from = c;

            sourceFile = from.x;
            sourceRank = from.y;
        }
    }
    else
    {
        if (movelen < 3)
            return false;

        // SAN move.

        // Check for castling moves.
        if (strcmp(_move, "O-O") == 0)
        {
            pieceType = Piece::King;

            if (turn == Side::White)
            {
                to.x = 6;
                to.y = 7;
            }
            else
            {
                to.x = 6;
                to.y = 0;
            }
        }
        else if (strcmp(_move, "O-O-O") == 0)
        {
            pieceType = Piece::King;

            if (turn == Side::White)
            {
                to.x = 2;
                to.y = 7;
            }
            else
            {
                to.x = 2;
                to.y = 0;
            }
        }
        else
        {
            switch (_move[0])
            {
            case 'K':
                pieceType = Piece::King;
                break;
            case 'Q':
                pieceType = Piece::Queen;
                break;
            case 'R':
                pieceType = Piece::Rook;
                break;
            case 'N':
                pieceType = Piece::Knight;
                break;
            case 'B':
                pieceType = Piece::Bishop;
                break;
            default:
                return false;
            }

            i = 1;

            // Check for disambiguation information.
            if (isdigit(_move[i]))
            {
                sourceRank = 7 - (_move[i] - '1');
                if (sourceRank < 0 || sourceRank > 7)
                    return false;
                ++i;
            }
            else if (_move[i] != 'x' && islower(_move[i]) && islower(_move[i+1]))
            {
                sourceFile = _move[i] - 'a';
                if (sourceFile < 0 || sourceFile > 7)
                    return false;
                ++i;
            }

            // Check where to continue parsing.
            if (_move[i] == 'x')
                ++i; // Ignore capture move indicator ('x').

            // Parse destination.
            to.x = _move[i] - 'a';
            if (to.x < 0 || to.x > 7)
                return false;
            to.y = 7 - (_move[i+1] - '1');
            if (to.y < 0 || to.y > 7)
                return false;
        }
    }

    // If the source file/rank was set, use these as the upper limit, so that
    // only that file/rank is checked for a piece which might be able to
    // perform the given move. This information is used to disambiguate moves.
    // Otherwise, if the source file/rank was not set, simply search the whole
    // board.
    if (sourceFile == -1)
        sourceFile = 0;
    else
        fileLimit = sourceFile;

    if (sourceRank == -1)
        sourceRank = 0;
    else
        rankLimit = sourceRank;

    // TODO If it was a CAN move, we have the 'from' coordinate too, so only
    //      check if 'from' contains a piece, and if so, check canMove(from, to).
    //      Also check if the 'from' coordinate contains a piece of the turn party.
    //      If not -> move is invalid.

    // TODO If it was a SAN move, we need to know the piece type, and only check
    //      for pieces of that type, because pieces of different types might be
    //      able to reach 'to'.
    //      Also check, when looping over the pieces and we've found a piece,
    //      whether that piece belongs to the turn party.
    //      If not -> move is invalid.

    if (isSanMove)
        assert(pieceType != Piece::None);

    // Iterate through the board to find the piece that can perform the move. If 'sourceRank' or
    // 'sourceFile' was set (for e.g., when disambiguation information was included in the move
    // symbol), we only iterate over those.
    assert(sourceRank >= 0 && sourceRank <= 7);
    assert(sourceFile >= 0 && sourceFile <= 7);
    assert(rankLimit >= 0 && rankLimit <= 7);
    assert(fileLimit >= 0 && fileLimit <= 7);
    for (int y = sourceRank; y <= rankLimit; ++y)
    {
        for (int x = sourceFile; x <= fileLimit; ++x)
        {
            const Piece &piece = board.pieces[x][y];

            if (piece.type == Piece::None || piece.side != turn)
                continue;
            if (pieceType != Piece::None && piece.type != pieceType)
                continue;

            Coord f = Coord(x, y);
            if (canMove(f, to))
            {
                from = f;
                return true;
            }
        }
    }

    // No piece was able to perform the move, move is invalid.
    return false;
}

void Rules::reset()
{
    board.reset();

    turn = Side::White;

    enPassantOpportunity = false;
    enPassant.reset();

    castlingFlags = 0;

    history.clear();
}

void Rules::switchTurn()
{
    turn = turn == Side::White ? Side::Black : Side::White;
}

Side::Type Rules::turnParty() const
{
    return turn;
}

void Rules::undoPly()
{
    Ply ply = history.undoPly();

    board.pieces[ply.from.x][ply.from.y] = Piece(ply.piece);
    if (ply.isEnPassant)
    {
        board.pieces[ply.ci.location.x][ply.ci.location.y] = Piece(ply.ci.piece);
        board.pieces[ply.to.x][ply.to.y].type = Piece::None;
    }
    else if (ply.isCastle)
    {
        board.pieces[ply.csi.rookSource.x][ply.csi.rookSource.y] =
            board.pieces[ply.csi.rookDestination.x][ply.csi.rookDestination.y];
        board.pieces[ply.csi.rookDestination.x][ply.csi.rookDestination.y].type = Piece::None;
        board.pieces[ply.to.x][ply.to.y].type = Piece::None;
    }
    else
    {
        if (ply.isCapture)
            board.pieces[ply.to.x][ply.to.y] = Piece(ply.ci.piece);
        else
            board.pieces[ply.to.x][ply.to.y].type = Piece::None;
    }

    if (history.plyCount() > 0)
    {
        const Ply &p = history.currentPly();

        enPassantOpportunity = p.enPassantOpportunity;
        enPassant = p.enPassant;

        castlingFlags = p.castlingFlags;
    }
    else
    {
        enPassantOpportunity = false;
        castlingFlags = 0;
    }

    switchTurn();
}

Side::Type Rules::opposingParty() const
{
    return turn == Side::White ? Side::Black : Side::White;
}

void Rules::__move(vector<vector<Piece> > &pieces, const Coord &from, const Coord &to,
                   const CaptureInfo &captureInfo, Piece::Type promotion) const
{
    assert((pieces[from.x][from.y].type != Piece::None) && "No piece present at the requested location");

    if (pieces[from.x][from.y].type == Piece::Pawn && (to.y == 0 || to.y == 7))
    {
        assert(promotion == Piece::Queen || promotion == Piece::Rook ||
               promotion == Piece::Bishop || promotion == Piece::Knight);
        pieces[to.x][to.y] = Piece(promotion, pieces[from.x][from.y].side);
    }
    else
    {
        // For capture moves the destination is not always equal to the location of the captured
        // piece (e.g., in En Passant moves, and perhaps in other variations of chess), that's why
        // we use CaptureInfo here.
        if (captureInfo.hasCapture())
            pieces[captureInfo.location.x][captureInfo.location.y].type = Piece::None;

        pieces[to.x][to.y] = pieces[from.x][from.y];
    }

    pieces[from.x][from.y].type = Piece::None;
}

bool Rules::canMakeAnyMove(const Coord &c) const
{
    assert(board.pieces[c.x][c.y].type != Piece::None);

    size_t i;
    const Piece &p = board.pieces[c.x][c.y];
    Piece::Type t = p.type;
    Side::Type s = p.side;

    if (t == Piece::Pawn)
    {
        // If we can make a two-step pawn move, we can also make a one-step
        // pawn move. Therefore, don't bother checking the two-step pawn move.
        if (canMove(c, Coord(c.x, c.y + (s == Side::White ? -1 : +1))))
            return true;

        // Now try the capture move. This covers en passant captures, pawn
        // captures that result in promotion, and of course regular captures.
        if (canMove(c, Coord(c.x + 1, c.y + (s == Side::White ? -1 : +1))))
            return true;
        if (canMove(c, Coord(c.x - 1, c.y + (s == Side::White ? -1 : +1))))
            return true;

        return false;
    }

    assert((unsigned)t < sizeof deltas / sizeof deltas[0]);
    assert((unsigned)t < sizeof deltaArrayLengths / sizeof deltaArrayLengths[0]);

    // Simply ignore castling, since if we can castle, we can also make other
    // moves, so we'd return 'true' anyway.
    for (i = 0; i < deltaArrayLengths[t]; ++i)
    {
        Coord dst(c.x + deltas[t][i].x, c.y + deltas[t][i].y);
        if (canMove(c, dst))
            return true;
        else
        {
            // If we can't move one step in a particular direction, it's of no use
            // to try further moves in that direction (since they too would be
            // illegal). So, we only need to check one move in each direction.
        }
    }

    return false;
}

bool Rules::canPathBeBlocked(const vector<vector<Piece> > &pieces,
        const Coord &from, const Coord &to, const Coord &v, Side::Type side) const
{
    int direction = getMovementDirection(v);
    int x, y;

    // Check if we're only moving one step, since the path can't be blocked
    // when only moving one step.
    if (abs(to.x - from.x) <= 1 && abs(to.y - from.y) <= 1)
        return false;

    /*
     * Travel along the path and see if it can be blocked, if it isn't already.
     */

    switch (direction)
    {
    case MR_PATH_N:
    {
        for (x = from.x, y = from.y - 1; y > to.y; --y)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_NW:
    {
        for (x = from.x - 1, y = from.y - 1; x > to.x; --x, --y)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_W:
    {
        for (x = from.x - 1, y = from.y; x > to.x; --x)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_SW:
    {
        for (x = from.x - 1, y = from.y + 1; y < to.y; --x, ++y)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_S:
    {
        for (x = from.x, y = from.y + 1; y < to.y; ++y)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_SE:
    {
        for (x = from.x + 1, y = from.y + 1; x < to.x; ++x, ++y)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_E:
    {
        for (x = from.x + 1, y = from.y; x < to.x; ++x)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    case MR_PATH_NE:
    {
        for (x = from.x + 1, y = from.y - 1; x < to.x; ++x, --y)
        {
            if (canSquareBeOccupied(pieces, Coord(x, y), side))
                return true;
        }
        break;
    }
    default:
        assert(0 && "Unknown direction");
        break;
    }

    return false;
}

/* This function is almost identical to isSquareAttacked(), although here the
 * move to the square does not need to be an attack-move. Thus, a pawn which
 * simply moves forward (as opposed to capturing) is also a valid way to
 * occupy a square. */
bool Rules::canSquareBeOccupied(const vector<vector<Piece> > &pieces,
        const Coord &square, Side::Type side) const
{
    /* Check if the square is already occupied. */
    if (pieces[square.x][square.y].type != Piece::None)
        return true;

    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            if (pieces[x][y].type == Piece::None || pieces[x][y].side != side)
                continue;

            if (canMove(Coord(x, y), square))
                return true;
        }
    }

    return false;
}

/* TODO *'captured_pieces'*
 *      This is where all the captures can easily be tracked, should we ever want to display them
 *      in the UI (so that users can see which pieces are captured).
 */
void Rules::capturePiece(vector<vector<Piece> > &pieces, const Coord &c)
{
    assert(pieces[c.x][c.y].type != Piece::None && "There is no piece to be captured at that location");
    pieces[c.x][c.y].type = Piece::None;
}

void Rules::checkDraws(Result &result) const
{
    if (isDrawByStalemate())
        result.draw = Result::DrawByStalemate;
    else if (isDrawByInsufficientMaterial())
        result.draw = Result::DrawByInsufficientMaterial;
    else
        result.draw = Result::NoDraw;
}

#define SLOPE_UNDEFINED 0xC0FFEE
int Rules::getMovementDirection(const Coord &v) const
{
    // Checking if the slope is valid is not the responsibility of this function.

    // Here we assume the result of the division is always in range of -1 to +1.
    // For a knight, it could be that the movement vector is x=2,y=-1, but knights fall outside of
    // the scope of the function.
    int slope = v.x ? v.y / v.x : SLOPE_UNDEFINED;

    switch (slope)
    {
    case -1:
    {
        if (v.x > 0 && v.y < 0)
            return MR_PATH_NE;
        return MR_PATH_SW;
    }
    case +1:
    {
        if (v.x < 0 && v.y < 0)
            return MR_PATH_NW;
        return MR_PATH_SE;
    }
    case 0:
    {
        if (v.x < 0)
            return MR_PATH_W;
        return MR_PATH_E;
    }
    case SLOPE_UNDEFINED:
    {
        if (v.y < 0)
            return MR_PATH_N;
        return MR_PATH_S;
    }
    default:
    {
        // Invalid slope. In chess, the slope shall be either undefined or in range of -1 to +1.
        assert(0 && "Slope should either be undefined or in range of -1 to +1");
        return 0;
    }
    }
}

bool Rules::isCastleMove(const Piece &piece, const Coord &to, const Coord &v) const
{
    assert(piece.type != Piece::None);
    return ((piece.type == Piece::King) &&
            ((abs(v.x) == 2) && (v.y == 0)) &&
            (((piece.side == Side::White) && ((to == Coord(6, 7)) || (to == Coord(2, 7)))) ||
             ((piece.side == Side::Black) && ((to == Coord(6, 0)) || (to == Coord(2, 0))))));
}

bool Rules::isCheckmate() const
{
    Coord attacker;
    bool  isAttackerKnight = false;
    int   numAttackers = 0;
    Coord opposingKing(-1, -1);

    // First, locate the king of the opponent.
    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            const Piece &piece = board.pieces[x][y];
            if (piece.type == Piece::King && piece.side != turn)
            {
                opposingKing.x = x;
                opposingKing.y = y;
                break;
            }
        }
    }

    // Surely there should be a king on the board.
    assert(opposingKing.x != -1 && opposingKing.y != -1);

    // Find the pieces of our side that are able to attack the opponent's king.
    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            const Piece &piece = board.pieces[x][y];
            if (piece.type == Piece::None || piece.side != turn)
                continue;

            Coord source(x, y);
            if (canMove(source, opposingKing))
            {
                attacker = source;
                isAttackerKnight = piece.type == Piece::Knight;
                ++numAttackers;
            }
        }
    }

    if (numAttackers == 0)
        return false;

    // Check if the opposing king can be moved to a safe square.
    for (size_t i = 0; i < sizeof kingDeltas / sizeof kingDeltas[0]; ++i)
    {
        Coord dest(opposingKing.x + kingDeltas[i].x,
                   opposingKing.y + kingDeltas[i].y);

        // Check if the destination is within the bounds of the board.
        if (board.isWithinBounds(dest))
        {
            // Check if the destination is safe (i.e., can't be attacked by the opponent).
            if (canMove(opposingKing, dest))
                return false;
        }
    }

    // If the king has no safe square to go to, and we have more than one attacker, it is
    // checkmate.
    if (numAttackers > 1)
        return true;

    // If there's only one attacker, check if it can be captured, or whether its line of attack can
    // be blocked.

    // Check if any of the opposing pieces can capture the attacking piece.
    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            const Piece &piece = board.pieces[x][y];
            if (piece.type == Piece::None || piece.side == turn)
                continue;

            if (canMove(Coord(x, y), attacker))
                return false;
        }
    }

    // Check if there actually is a line of attack (i.e., there isn't when the attacking piece is
    // adjacent to the king), and whether it can be blocked. If it is a knight, there is no line of
    // attack (it's a direct attack), so it's checkmate unless it can be captured.
    if (!isAttackerKnight)
    {
        Coord movementVector(opposingKing.x - attacker.x, opposingKing.y - attacker.y);
        if (canPathBeBlocked(board.pieces, attacker, opposingKing, movementVector, opposingParty()))
            return false;
    }

    return true;
}

// The game is considered to be a draw by insufficient material only under any of the following
// conditions:
//     - Both sides only have a king.
//     - One side only has a king and bishop, the other only a king.
//     - One side only has a king and knight, the other only a king.
//     - Both sides only have a king and bishops of the same type.
bool Rules::isDrawByInsufficientMaterial() const
{
    int knights[2] = { 0, 0 };
    bool hasLightSquareBishop[2] = { false, false },
         hasDarkSquareBishop[2] = { false, false };
    bool isLightSquare = true;

    // These are going to be used as indices into the above arrays.
    assert(Side::White == 0);
    assert(Side::Black == 1);

    for (int y = 0; y < board.height; ++y)
    {
        isLightSquare = !isLightSquare;

        for (int x = 0; x < board.width; ++x)
        {
            isLightSquare = !isLightSquare;

            const Piece &piece = board.pieces[x][y];
            if (piece.type == Piece::None)
                continue;

            int t = piece.type;

            if (t == Piece::Knight)
                knights[piece.side]++;
            else if (t == Piece::Bishop)
            {
                if (isLightSquare)
                    hasLightSquareBishop[piece.side] = true;
                else
                    hasDarkSquareBishop[piece.side] = true;
            }
            else if ((t == Piece::Queen) || (t == Piece::Rook) || (t == Piece::Pawn))
            {
                // At least one side has mating material or potential mating material.
                return false;
            }
        }
    }

    // Do the following checks for both sides.
    for (int side = 0; side < 2; ++side)
    {
        int otherSide = side ^ 1;
        bool otherSideHasNothing = (knights[otherSide] == 0) &&
                                   (!hasLightSquareBishop[otherSide] &&
                                    !hasDarkSquareBishop[otherSide]),
             sideHasOnlyOneTypeOfBishop = hasLightSquareBishop[side] ^ hasDarkSquareBishop[side],
             otherSideHasOnlyOneTypeOfBishop = hasLightSquareBishop[otherSide] ^ hasDarkSquareBishop[otherSide];

        // Do both sides have no knights, and only bishops of the same type (that is, the bishops
        // of one side are of the same type as those of the other)?
        if ((knights[side] == 0) &&
            sideHasOnlyOneTypeOfBishop &&
            (knights[otherSide] == 0) &&
            otherSideHasOnlyOneTypeOfBishop &&
            // If we know for sure that both sides have bishops, and only bishops of the same type
            // (individually, that is), then the following comparison tells us whether the bishops
            // of both sides are of the same type, since if the comparison is true, then it implies
            // that 'hasDarkSquareBishop[side] == hasDarkSquareBishop[otherSide]' is true as well,
            // as the 'hasDarkSquareBishop' bishop array will contain zeroes if the
            // 'hasLightSquareBishop' array contains ones, and vice versa.
            (hasLightSquareBishop[side] == hasLightSquareBishop[otherSide]))
        {
            return true;
        }
        // Does one side only have one or fewer knights and no bishops, and the other nothing?
        else if ((knights[side] <= 1) &&
                 (!hasLightSquareBishop[side] && !hasDarkSquareBishop[side]) &&
                 otherSideHasNothing)
        {
            return true;
        }
        // Does one side have no knights and no bishop pairs (in other words, does one side have no
        // knights and no bishops or only bishops of the same type), and the other nothing?
        else if ((knights[side] == 0) &&
                 !(hasLightSquareBishop[side] && hasDarkSquareBishop[side]) &&
                 otherSideHasNothing)
        {
            return true;
        }
    }

    return false;
}

bool Rules::isDrawByStalemate() const
{
    if (isKingInCheck(board.pieces, Side::opposite(turn)))
        return false; // Not a stalemate, perhaps a checkmate, or no game ending.

    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            const Piece &piece = board.pieces[x][y];
            if (piece.type != Piece::None && piece.side != turn)
            {
                if (canMakeAnyMove(Coord(x, y)))
                    return false;
            }
        }
    }

    return true;
}

bool Rules::isKingInCheck(const vector<vector<Piece> > &pieces, Side::Type side) const
{
    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            if (pieces[x][y].type == Piece::King && pieces[x][y].side == side)
            {
                if (isSquareAttacked(pieces, Coord(x, y), Side::opposite(side)))
                    return true;
            }
        }
    }

    return false;
}

bool Rules::isPathBlocked(const vector<vector<Piece> > &pieces,
        const Coord &from, const Coord &to, const Coord &v) const
{
    int direction = getMovementDirection(v);
    int x, y;

    // Check if we're only moving one step, since the path can't be blocked
    // when only moving one step.
    if (abs(to.x - from.x) <= 1 && abs(to.y - from.y) <= 1)
        return false;

    /*
     * Travel along the path and see if any piece is in the way.
     */

    switch (direction)
    {
    case MR_PATH_N:
    {
        for (x = from.x, y = from.y - 1; y > to.y; --y)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_NW:
    {
        for (x = from.x - 1, y = from.y - 1; x > to.x; --x, --y)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_W:
    {
        for (x = from.x - 1, y = from.y; x > to.x; --x)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_SW:
    {
        for (x = from.x - 1, y = from.y + 1; y < to.y; --x, ++y)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_S:
    {
        for (x = from.x, y = from.y + 1; y < to.y; ++y)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_SE:
    {
        for (x = from.x + 1, y = from.y + 1; x < to.x; ++x, ++y)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_E:
    {
        for (x = from.x + 1, y = from.y; x < to.x; ++x)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    case MR_PATH_NE:
    {
        for (x = from.x + 1, y = from.y - 1; x < to.x; ++x, --y)
        {
            if (pieces[x][y].type != Piece::None)
                return true;
        }
        break;
    }
    default:
        assert(0 && "Unknown direction");
        break;
    }

    return false;
}

// Checks whether a move is legal according to the rules of piece movement.
bool Rules::isPseudoLegalMove(const Piece &piece, const Coord &from, const Coord &v, PseudoMove &pseudoMove) const
{
    bool movingDiagonally = false, movingStraightLine = false;

    // Reset PseudoMove.
    pseudoMove = PseudoMove();

    if (v.x == 0 && v.y == 0)
        assert(0 && "Not a move, this case should be catched earlier on");

    movingStraightLine = v.x == 0 || v.y == 0;
    movingDiagonally = abs(v.y) == abs(v.x);

    switch (piece.type)
    {
    case Piece::King:
        return !(abs(v.x) > 1 || abs(v.y) > 1);

    case Piece::Queen:
        return movingStraightLine || movingDiagonally;

    case Piece::Rook:
        return movingStraightLine;

    case Piece::Knight:
        return (abs(v.x) == 2 && abs(v.y) == 1) || (abs(v.x) == 1 && abs(v.y) == 2);

    case Piece::Bishop:
        return movingDiagonally;

    case Piece::Pawn:
    {
        /*
         * First check the exceptions.
         */

        // Pawns can't move backwards.
        if (piece.side == Side::White && v.y > 0)
            return false;
        else if (piece.side == Side::Black && v.y < 0)
            return false;

        // Movement on the X axis is permitted to capture, in which the X axis vector must be equal
        // to one. Anything higher is always invalid.
        if (abs(v.x) > 1)
            return false;

        /*
         * Now check the legality.
         */

        // Can move two steps at first, check if it's requested and legal.
        if (((v.x == 0) && (abs(v.y) == 2)) &&
            (from.y == (piece.side == Side::White ? (Board::DefaultHeight - 2) : 1)))
        {
            pseudoMove.isTwoStepPawnMove = true;
            return true;
        }
        else // Otherwise, see if we are moving one step.
        {
            // Check if we're doing a 'pawn capture' move.
            if (abs(v.x) == 1)
                pseudoMove.isPawnCapture = true;

            return abs(v.y) == 1;
        }

        /* NOTREACHED */
        assert(0);
    }

    default:
        assert(0 && "Unhandled piece type in switch");
        return false; // Return something, to prevent a compiler warning.
    }
}

bool Rules::isSquareAttacked(const vector<vector<Piece> > &pieces, const Coord &square, Side::Type attacker) const
{
    PseudoMove pseudoMove;

    // Iterate through the board to check if any of the pieces can attack the square.
    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            // Ignore empty squares and pieces of the opponent.
            if (pieces[x][y].type == Piece::None || pieces[x][y].side != attacker)
                continue;

            // A piece can't attack itself.
            if (square.x == x && square.y == y)
                continue;

            Coord movementVector(square.x - x, square.y - y);
            assert(movementVector.x || movementVector.y);

            // Now check if it can reach the square we're evaluating.
            if (isPseudoLegalMove(pieces[x][y], Coord(x, y), movementVector, pseudoMove))
            {
                if (pieces[x][y].type == Piece::Knight)
                    return true; // For knights, paths can't be blocked. The knight can attack.
                else if (pieces[x][y].type == Piece::Pawn)
                {
                    if (pseudoMove.isPawnCapture)
                        return true;
                }
                else if (!isPathBlocked(pieces, Coord(x, y), square, movementVector))
                    return true;
            }
        }
    }

    return false;
}

void Rules::needDisambiguation(const vector<vector<Piece> > &pieces, const Coord &from, const Coord &to, bool &file, bool &rank) const
{
    assert(pieces[from.x][from.y].type != Piece::None);

    const Piece &piece = pieces[from.x][from.y];
    Piece::Type t = piece.type;
    Side::Type side = piece.side;

    if (t == Piece::Pawn)
        return;

    for (int y = 0; y < board.height; ++y)
    {
        for (int x = 0; x < board.width; ++x)
        {
            const Piece &p = pieces[x][y];

            if ((p.type != Piece::None) &&
                ((from.x != x) || (from.y != y)) &&
                p.side == side &&
                p.type == t &&
                canMove(Coord(x, y), to))
            {
                if (x == from.x)
                    rank = true;
                else
                {
                    /* Either the rank is ambiguous, or it doesn't matter
                     * whether we choose to disambiguate using the file/rank.
                     * So, simply choose to disambiguate using the file. */
                    file = true;
                }
                return;
            }
        }
    }
}
