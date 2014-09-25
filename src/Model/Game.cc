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

#include "Game.hh"
#include "IGameListener.hh"
#include <cassert>

Game::Game()
    : listener(0)
{
    reset();
}

void Game::reset(Side::Type engineSide)
{
    rules.reset();
    moves_.clear();
    setMutation(false);
    result_.reset();

    whiteName = "-";
    blackName = "-";
    event = "-";

    // NOTE: 'engineSide' may be Side::None when engines are disabled.
    whiteIsEngine = engineSide == Side::White;
    blackIsEngine = engineSide == Side::Black;
}

const Board &Game::board() const
{
    return rules.board;
}

bool Game::canUndoMove() const
{
    return rules.canUndoMove();
}

bool Game::canUndoPly() const
{
    return rules.canUndoPly();
}

const Piece &Game::getPiece(int x, int y) const
{
    assert(isWithinBounds(Coord(x, y)));
    return rules.board.pieces[x][y];
}

Side::Type Game::humanParty() const
{
    if (whiteIsEngine && !blackIsEngine)
        return Side::Black;
    else if (blackIsEngine && !whiteIsEngine)
        return Side::White;
    else
    {
        assert(0);
        return Side::None;
    }
}

// Returns true iff the game has at least one ply and is not over yet (i.e., has no result).
bool Game::isInProgress() const
{
    // TODO *'redo'* (for if we implement 'redo')
    // The game should also be considered 'in progress' if there are moves in the redo history, as
    // it may be that the user is going to redo some moves. In other words, there's the potential
    // for the game to still be in progress.
    return moves().plyCount() != 0 && !result().hasResult();
}

bool Game::isMutated() const
{
    return isMutated_;
}

void Game::setMutation(bool b)
{
    isMutated_ = b;

    if (listener)
        listener->mutationChange(b);
}

bool Game::isKingInCheck(Side::Type side) const
{
    return rules.isKingInCheck(side);
}

bool Game::isOppositePartyPiece(const Coord &c, Side::Type side) const
{
    if (rules.board.pieces[c.x][c.y].type == Piece::None)
        return false;

    return rules.board.pieces[c.x][c.y].side == Side::opposite(side);
}

bool Game::isTurnPartyPiece(const Coord &c) const
{
    if (rules.board.pieces[c.x][c.y].type == Piece::None)
        return false;

    return rules.board.pieces[c.x][c.y].side == rules.turnParty();
}

bool Game::isPieceAt(int x, int y) const
{
    return rules.board.pieces[x][y].type != Piece::None;
}

bool Game::isPromotion(const Coord &from, const Coord &to) const
{
    return rules.isPromotion(from, to);
}

std::string Game::toPGN() const
{
    std::string gameTermination;
    if (result_.hasResult())
    {
        if (result_.winner != Side::None)
            gameTermination = result_.winner == Side::White ? "1-0" : "0-1";
        else
            gameTermination = "1/2-1/2";
    }
    else
        gameTermination = "*";
    assert(gameTermination.length());

    std::string tagSection;
    tagSection += ("[Event \"" + event + "\"]\n").toUtf8().constData();
    tagSection += "[Site \"-\"]\n";
    tagSection += "[Date \"-\"]\n";
    tagSection += "[Round \"-\"]\n";
    tagSection += ("[White \"" + whiteName + "\"]\n").toUtf8().constData();
    tagSection += ("[Black \"" + blackName + "\"]\n").toUtf8().constData();
    tagSection += "[Result \"" + gameTermination + "\"]\n";
    if (whiteIsEngine)
        tagSection += "[WhiteType \"program\"]\n";
    if (blackIsEngine)
        tagSection += "[BlackType \"program\"]\n";
    assert(tagSection.length());
    return tagSection + "\n" + (moves_.plyCount() ? (moves_.toString() + "\n") : "") + gameTermination + "\n\n";
}

const PgnMoveList &Game::moves() const
{
    return moves_;
}

Result Game::result() const
{
    return result_;
}

void Game::setResult(const Result &_result)
{
    result_ = _result;
    setMutation(true);
}

void Game::setListener(IGameListener &_listener)
{
    listener = &_listener;
}

void Game::setSideIsEngine(Side::Type side, bool isEngine)
{
    if (side == Side::None)
        return;

    if (side == Side::White)
        whiteIsEngine = isEngine;
    else
    {
        assert(side == Side::Black);
        blackIsEngine = isEngine;
    }

    // Commented out because the mutation state should merely reflect whether
    // the moves in the game were changed.
    //setMutation(true);
}

bool Game::isWithinBounds(const Coord &c) const
{
    return rules.board.isWithinBounds(c);
}

bool Game::canMove(const Coord &from, const Coord &to) const
{
    return rules.canMove(from, to);
}

bool Game::move(const Coord &from, const Coord &to, Piece::Type promotion, Rules::ReasonInvalid *reasonInvalid /* = 0 */)
{
    Ply  ply;
    bool r;

    r = rules.move(from, to, promotion, ply, result_, true, reasonInvalid);

    if (r)
    {
        moves_.addPly(ply);
        setMutation(true);
    }
    return r;
}

Side::Type Game::opposingParty() const
{
    return rules.opposingParty();
}

bool Game::parseMove(Coord &from, Coord &to, const char *_move,
                     bool &isPawnPromotion, Piece::Type &promotion) const
{
    return rules.parseMove(from, to, _move, isPawnPromotion, promotion);
}

Side::Type Game::turnParty() const
{
    return rules.turnParty();
}

void Game::undoPly()
{
    rules.undoPly();
    moves_.undoPly();
    setMutation(true);
    result_.reset();
}
