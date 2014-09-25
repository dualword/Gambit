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

#ifndef QUOT_H
#define QUOT_H

// Example:
//     #define FOOBAR hello
//     _QUOT(FOOBAR) -> "FOOBAR"
//     QUOT(FOOBAR) -> "hello"
// How this works:
//     QUOT(x) uses the _QUOT(x) macro to expand the value of 'x', such that
//     _QUOT will add double quotes to the value of 'x', rather than to the
//     alias 'x' itself.
#define _QUOT(x) #x
#define QUOT(x) _QUOT(x)

#endif
