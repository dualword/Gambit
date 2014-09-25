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

#ifndef PLY_HH
#define PLY_HH

#include "Coord.hh"
#include "CaptureInfo.hh"
#include "CastlingFlags.hh"
#include "CastlingInfo.hh"
#include "EnPassant.hh"
#include "Piece.hh"
#include <string>

class Ply
{
public:
    enum StringConversionType
    {
        ToCanString,
        ToSanString
    };

    Ply();

    Coord from;
    Coord to;

    Piece piece;

    Piece::Type promotion;

    bool isCapture;
    CaptureInfo ci;

    bool isCheck;
    bool isCheckmate;

    bool disambiguateFile;
    bool disambiguateRank;

    bool isCastle;
    CastlingInfo csi;
    int castlingFlags;

    bool isEnPassant;
    bool enPassantOpportunity;
    EnPassant enPassant;

    bool isKingsideCastle() const;
    bool isQueensideCastle() const;

    std::string toCAN() const;
    std::string toSAN() const;
    std::string toString(StringConversionType t = ToSanString) const;
};

#endif
