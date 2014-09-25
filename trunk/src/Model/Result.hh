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

#ifndef RESULT_HH
#define RESULT_HH

#include "Side.hh"

class Result
{
public:
    enum DrawReason
    {
        NoDraw,
        //DrawByAgreement, TODO *'draw'* (dont forget to grep the source code for other occurrences of *'draw'*) use this one when a draw offer is accepted
        DrawByInsufficientMaterial,
        DrawByStalemate
    };

    Result();
    void reset();
    bool hasResult() const;

    Side::Type winner;
    bool isCheckmate;
    DrawReason draw;
    bool gameEndedByResignation;
};

#endif
