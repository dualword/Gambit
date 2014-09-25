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

#include "enforce.hh"
#include "GeneralException.hh"

void _enforce(const char *expr, const char *file, int line)
{
    throw GeneralException("Enforcement failed: %s, file %s, line %d.", expr, file, line);
}
