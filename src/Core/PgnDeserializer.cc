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

#include "PgnDeserializer.hh"
#include "debugf.h"
#include "Model/PgnDatabase.hh"
#include "Model/PgnPlayerType.hh"
#include "Model/Result.hh"
#include <QChar>
#include <QFile>
#include <QTextStream>
#include <cassert>

bool PgnDeserializer::isContinuationChar(const QChar &ch)
{
    return ch.isLetterOrNumber() ||
        (ch == '_' || ch == '+' || ch == '#' ||
         ch == '=' || ch == ':' || ch == '-');
}

bool PgnDeserializer::parseGame(QTextStream &stream, Game &game)
{
    if (!parseTagSection(stream, game))
        return false;

    if (!parseMoveSection(stream, game))
        return false;

    return true;
}

// This function doesn't parse the '*' game-termination. It is assumed that
// this will be handled elsewhere.
bool PgnDeserializer::parseGameTermination(QTextStream &stream, Game &game)
{
    // TODO
    (void)stream;(void)game;
    bool rval = false;
    qint64 pos;
    (void)pos;

    return rval;
}

// TODO: check if this function ever returns false ... if not then make its return type 'void'
// TODO: When we detect a 1 or 0
//       (which could be the beginning of a game termination like 1-0, 0-1, or 1/2-1/2), try
//       parseGameTermation() to parse it. If it fails, just continue parsing.
bool PgnDeserializer::parseMoveSection(QTextStream &stream, Game &game)
{
    qint64 pos;
    QString sym;

    while (!stream.atEnd())
    {
        stream.skipWhiteSpace();

        sym.clear();
        if (parseSymbol(stream, sym))
        {
            bool        isPawnPromotion;
            Coord       from, to;
            Piece::Type promotion = Piece::None;

            // Try parsing the symbol as a SAN move. If this fails, we ignore
            // the symbol. That way, we also automatically ignore move number
            // indications.

            if (game.parseMove(from, to, sym.toUtf8().constData(), isPawnPromotion, promotion))
            {
                debugf("move successfully parsed: %s\n", sym.toUtf8().constData());

                // If the move was parsed, then it must be a valid move, since
                // for a move to be parsed, we have to check whether it is a
                // valid move. Therefore, move() should always return true.
                bool r = game.move(from, to, promotion);
                assert(r == true);
                (void)r;

                // TODO: Test whether move() fails when the game has already
                //       ended due to a checkmate/draw.
            }
            else
            {
                debugf("couldn't parse move, symbol token was '%s'\n", sym.toUtf8().constData());
                // TODO: fail and display a descriptive error message
            }
        }
        else
        {
            QChar ch;
            pos = stream.pos();
            stream >> ch;

            if (ch == '*')
            {
                // Game-termination detected.
                // TODO: set game result to 'in-progress'
                return true;
            }
            else if (ch == '[')
            {
                // Tag-section detected.
                stream.seek(pos);
                return true;
            }
            else if (ch == '.')
            {
                // Ignore the period.
            }
            else if (ch == '(')
            {
                // TODO: parse recursive variation
            }
            else if (ch == '{')
            {
                // TODO: parse and save the comment
            }
            else if (ch == '<')
            {
                // TODO: skip until after '>', this is reserved by the PGN specification for future use
            }
            else if (ch == '$')
            {
                // TODO: ignore/skip numeric annotation glyph
            }
            else
            {
                // Ignore the unrecognized token. Only stop grabbing characters
                // if a recognized token is found that is not part of an
                // element-sequence.
            }
        }
    }

    return true;
}

bool PgnDeserializer::parseString(QTextStream &stream, QString &value)
{
    bool rval = false;
    bool seek = true;
    qint64 pos;
    QChar ch;
    bool isEscapeChar = false;

    pos = stream.pos();
    stream >> ch;
    if (ch != '"')
        goto done;

    for (;;)
    {
        if (stream.atEnd())
        {
            seek = false;
            break;
        }

        pos = stream.pos();
        stream >> ch;

        if (isEscapeChar)
        {
            value += ch;
            isEscapeChar = false;
            continue;
        }
        else
            isEscapeChar = ch == '\\';

        if (isEscapeChar)
            continue;
        else if (ch == '"')
        {
            // Return success iff the closing quote was found.
            rval = true;
            seek = false;
            break;
        }
        else
        {
            // TAB characters not allowed in strings by the PGN specification.
            if (ch != '\t')
                value += ch;
        }
    }

    debugf("parsed tag value=%s\n", value.toUtf8().constData());

done:
    if (seek)
        stream.seek(pos);
    return rval;
}

bool PgnDeserializer::parseSymbol(QTextStream &stream, QString &sym)
{
    bool rval = false;
    bool seek = true;
    qint64 pos;
    QChar ch;

    pos = stream.pos();
    stream >> ch;
    if (!ch.isLetterOrNumber())
        goto done;
    sym += ch;

    for (;;)
    {
        if (stream.atEnd())
        {
            seek = false;
            break;
        }

        pos = stream.pos();
        stream >> ch;
        if (!isContinuationChar(ch))
            break;

        sym += ch;
    }

    rval = true;

done:
    if (seek)
        stream.seek(pos);
    if (rval)
        debugf("sym=[%s]\n", sym.toUtf8().constData());
    return rval;
}

// Returns failure only when a parse error occurred *inside* a tag section.
bool PgnDeserializer::parseTagSection(QTextStream &stream, Game &game)
{
    bool rval = false;
    bool seek = true;
    qint64 pos;
    QChar ch;
    QString tagName;
    QString tagValue;

    for (;;)
    {
        if (stream.atEnd())
        {
            rval = true;
            seek = false;
            break;
        }

        stream.skipWhiteSpace();

        pos = stream.pos();
        stream >> ch;
        if (ch != '[')
        {
            rval = true;
            break;
        }

        tagName.clear();
        stream >> tagName; // TODO: what if we have no more data left after this?
        stream.skipWhiteSpace();

        debugf("parsed tag name=%s\n", tagName.toUtf8().constData());

        tagValue.clear();
        if (!parseString(stream, tagValue))
            break;

        if (tagName == "Event")
        {
            game.event = tagValue;
        }
        else if (tagName == "Date")
        {
            // TODO: store date
        }
        else if (tagName == "White")
        {
            game.whiteName = tagValue;
        }
        else if (tagName == "Black")
        {
            game.blackName = tagValue;
        }
        else if (tagName == "WhiteType")
        {
            game.whiteType = PgnPlayerType(tagValue.toUtf8().constData());
        }
        else if (tagName == "BlackType")
        {
            game.blackType = PgnPlayerType(tagValue.toUtf8().constData());
        }
        else if (tagName == "Round")
        {
            // TODO: store round
        }
        else if (tagName == "Result")
        {
            // TODO: perform result checking, and store the result ...
            // TODO: also detect inconsistencies between this result and the game-termination
        }
        else if (tagName == "Site")
        {
            // TODO: store site
        }
        else
        {
            // TODO: Perhaps store unrecognized tags in an std::map. When that
            //       is implemented, also use these tags when writing a PGN
            //       file so that all unrecognized tags that were read are also
            //       written.
        }

        pos = stream.pos();
        stream >> ch;
        if (ch != ']')
        {
            rval = false;
            break;
        }
    }

    if (seek)
        stream.seek(pos);
    return rval;
}

bool PgnDeserializer::load(const QString &fileName, PgnDatabase &db)
{
    bool rval = false;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    for (;;)
    {
        stream.skipWhiteSpace();
// TODO: remove
{
    QString bla;
    qint64 pos = stream.pos();
    stream >> bla;
    debugf("data @ pos %d = [%s]\n", (int)pos, bla.toUtf8().constData());
    stream.seek(pos);
}
        if (stream.atEnd())
        {
            rval = true;
            break;
        }

        Game game;
        if (!parseGame(stream, game))
        {
            // TODO: A parse error occurred, return a descriptive error so we
            //       can notify the user about what's wrong.
            break;
        }
        else
            db.games.push_back(game);
    }

    file.close();
    return rval;
}
