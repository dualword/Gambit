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

#include <exception>
#include <QString>

namespace Utils
{
namespace Qt
{

class QString_find_first_not_of_exception : public std::exception
{
public:
    QString_find_first_not_of_exception(const QString &_msg);
    ~QString_find_first_not_of_exception() throw();
    const char *what() const throw();

private:
    QString error;
};

QString_find_first_not_of_exception::QString_find_first_not_of_exception(const QString &_error)
    : error(_error)
{
}

QString_find_first_not_of_exception::~QString_find_first_not_of_exception() throw()
{
}

const char *QString_find_first_not_of_exception::what() const throw()
{
    return qPrintable(error);
}

/* Equivalent of the STL's std::string::find_first_not_of algorithm for
 * QString.
 *
 * Returns:
 *   On success, the position of the first character in 'subject' that is not
 *   in the sequence 'sequence'.
 *   On failure, the value -1.
 */
int QString_find_first_not_of(const QString &subject, const QString &sequence, int pos /* = 0 */)
{
    if (subject.length() == 0 || sequence.length() == 0)
        throw QString_find_first_not_of_exception("'subject' or 'sequence' parameter had a zero length");

    QString::const_iterator subjIt = subject.constBegin(),
                            subjEnd = subject.constEnd();
    int i = 0;

    for (i = 0; i < pos; ++i)
    {
        ++subjIt;
        if (subjIt == subjEnd)
            throw QString_find_first_not_of_exception("index 'pos' out of bounds");
    }

    for ( ; subjIt != subjEnd; ++subjIt)
    {
        QString::const_iterator seqIt = sequence.constBegin(),
                                seqEnd = sequence.constEnd();

        for ( ; seqIt != seqEnd; ++seqIt)
        {
            if (*subjIt == *seqIt)
                break;
        }

        if (seqIt == seqEnd)
            return i;

        i++;
    }

    return -1;
}

} /* namespace Qt */
} /* namespace Utils */
