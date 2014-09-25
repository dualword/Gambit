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

// Unlike assert(), enforce() is not meant to be turned on/off with the NDEBUG
// macro. Hence, an inclusion-guard should be used as usual.
#ifndef ENFORCE_HH
#define ENFORCE_HH

#define enforce(expr) (void)((expr) ? (void)0 : _enforce(#expr, __FILE__, __LINE__))

void _enforce(const char *expr, const char *file, int line);

#endif
