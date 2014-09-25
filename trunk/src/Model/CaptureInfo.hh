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

#ifndef CAPTURE_INFO_HH
#define CAPTURE_INFO_HH

#include "Coord.hh"
#include "Piece.hh"

class CaptureInfo
{
public:
    CaptureInfo();
    bool hasCapture() const;

    Coord location; // Location of the piece that was / is going to be captured.
    Piece piece; // Piece to be captured / captured piece.
};

#endif
