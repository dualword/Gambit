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

#include "Ply.hh"
#include <cassert>

Ply::Ply()
    : promotion(Piece::None), isCapture(false), isCheck(false), isCheckmate(false),
      disambiguateFile(false), disambiguateRank(false), isCastle(false), castlingFlags(0),
      enPassantOpportunity(false)
{
}

bool Ply::isKingsideCastle() const
{
    return (piece.type == Piece::King &&
            from.x == 4 &&
            (from.y == 0 || from.y == 7) &&
            to.x == 6 &&
            (to.y == 0 || to.y == 7));
}

bool Ply::isQueensideCastle() const
{
    return (piece.type == Piece::King &&
            from.x == 4 &&
            (from.y == 0 || from.y == 7) &&
            to.x == 2 &&
            (to.y == 0 || to.y == 7));
}

std::string Ply::toCAN() const
{
#define CAN_MOVE_BUF_SIZE 6
    char s[CAN_MOVE_BUF_SIZE];
    int i = 0;

    s[i++] = 'a' + from.x;
    s[i++] = '8' - from.y;
    s[i++] = 'a' + to.x;
    s[i++] = '8' - to.y;

    if (promotion != Piece::None)
    {
        switch (promotion)
        {
        case Piece::Queen : s[i++] = 'q'; break;
        case Piece::Rook  : s[i++] = 'r'; break;
        case Piece::Bishop: s[i++] = 'b'; break;
        case Piece::Knight: s[i++] = 'n'; break;
        default:
        {
            assert(0);
            /* NOTREACHED */
            break;
        }
        }
    }

    assert(i < CAN_MOVE_BUF_SIZE);
    s[i] = '\0';

    return std::string(s);
}

std::string Ply::toSAN() const
{
#define SAN_MOVE_BUF_SIZE 8
    char s[SAN_MOVE_BUF_SIZE];
    int i = 0;

    if (isKingsideCastle())
    {
        s[i++] = 'O';
        s[i++] = '-';
        s[i++] = 'O';
    }
    else if (isQueensideCastle())
    {
        s[i++] = 'O';
        s[i++] = '-';
        s[i++] = 'O';
        s[i++] = '-';
        s[i++] = 'O';
    }
    else
    {
        switch (piece.type)
        {
        case Piece::None  : return "";
        case Piece::King  : s[i++] = 'K'; break;
        case Piece::Queen : s[i++] = 'Q'; break;
        case Piece::Rook  : s[i++] = 'R'; break;
        case Piece::Bishop: s[i++] = 'B'; break;
        case Piece::Knight: s[i++] = 'N'; break;
        case Piece::Pawn:
        {
            if (isCapture)
                s[i++] = 'a' + from.x;
            break;
        }
        default:
            assert(0 && "Invalid piece type");
            break;
        }

        assert(!(disambiguateFile & disambiguateRank));
        if (disambiguateFile)
            s[i++] = 'a' + from.x;
        else if (disambiguateRank)
            s[i++] = '8' - from.y;

        if (isCapture)
            s[i++] = 'x';

        s[i++] = 'a' + to.x;
        s[i++] = '8' - to.y;

        if (promotion != Piece::None)
        {
            s[i++] = '=';

            switch (promotion)
            {
            case Piece::Queen : s[i++] = 'Q'; break;
            case Piece::Rook  : s[i++] = 'R'; break;
            case Piece::Bishop: s[i++] = 'B'; break;
            case Piece::Knight: s[i++] = 'N'; break;
            default:
            {
                assert(0);
                /* NOTREACHED */
                break;
            }
            }
        }
    }

    if (isCheckmate)
        s[i++] = '#';
    else if (isCheck)
        s[i++] = '+';

    assert(i < SAN_MOVE_BUF_SIZE);
    s[i] = '\0';

    return std::string(s);
}

std::string Ply::toString(StringConversionType t /* = ToSanString */) const
{
    return t == ToSanString ? toSAN() : toCAN();
}
