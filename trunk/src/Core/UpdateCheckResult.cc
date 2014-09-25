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

#include "UpdateCheckResult.hh"

UpdateCheckResult::UpdateCheckResult()
    : code_(Error)
{
}

UpdateCheckResult::UpdateCheckResult(int _code, const QString &_message /* = QString() */)
    : code_(_code),
      message_(_message)
{
}

int UpdateCheckResult::code() const
{
    return code_;
}

QString UpdateCheckResult::message() const
{
    return message_;
}
