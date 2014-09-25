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

/*
 * The languageIdString() function can be used to parse QLocale name strings such as 'nl_NL' and
 * 'de_DE', to extract the first 2 characters.
 * The function checks whether the locale name string associated with the language is indeed
 * recognized as a locale name string. This allows us to catch changes in the Qt toolkit, as we can
 * be sure that when we extract the first 2 characters, that they indeed represent the language ID.
 */

#include <QLocale>
#include <cassert>

namespace Utils
{
namespace Qt
{

static bool isLowerAnsii(QChar c)
{
    return c >= 'a' && c <= 'z';
}

static bool isUpperAnsii(QChar c)
{
    return c >= 'A' && c <= 'Z';
}

static bool isRecognizedLocaleName(const QString &localeName)
{
    if (localeName.length() < 2)
        return false;

    if (!isLowerAnsii(localeName[0]))
        return false;
    if (!isLowerAnsii(localeName[1]))
        return false;

    if (localeName.length() == 2)
        return true;
    else if (localeName.length() == 5)
    {
        if (localeName[2] != '_')
            return false;
        if (!isUpperAnsii(localeName[3]))
            return false;
        if (!isUpperAnsii(localeName[4]))
            return false;

        return true;
    }

    return false;
}

QString languageIdString(QLocale::Language language)
{
    const QString &s = QLocale(language).name();
    if (!isRecognizedLocaleName(s))
        assert(0);
    return s.mid(0, 2);
}

} /* namespace Qt */
} /* namespace Utils */
