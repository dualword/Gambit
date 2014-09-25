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

#ifndef UPDATE_CHECK_RESULT_HH
#define UPDATE_CHECK_RESULT_HH

#include <QString>

class UpdateCheckResult
{
public:
    enum
    {
        Error,
        UpdateAvailable,
        NoUpdateAvailable
    };

    UpdateCheckResult();
    UpdateCheckResult(int code, const QString &message = QString());

    int code() const;
    QString message() const;

private:
    int code_;
    QString message_;
};

#endif
