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

#ifndef PGN_DESERIALIZER_HH
#define PGN_DESERIALIZER_HH

class Game;
class QChar;
class QString;
class QTextStream;
struct PgnDatabase;

class PgnDeserializer
{
public:
    static bool load(const QString &, PgnDatabase &);

private:
    static bool isContinuationChar(const QChar &);
    static bool parseGame(QTextStream &, Game &);
    static bool parseGameTermination(QTextStream &, Game &);
    static bool parseMoveSection(QTextStream &, Game &);
    static bool parseString(QTextStream &, QString &);
    static bool parseSymbol(QTextStream &, QString &);
    static bool parseTagSection(QTextStream &, Game &);
};

#endif
