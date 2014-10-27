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

#ifndef PROJECT_INFO_H
#define PROJECT_INFO_H

#define AUTHOR                  "Jelle Geerts"

#define APP_UNIQUE_ID           0x2ED22A63

// For the (automatic) update system.
// An update should be available if the integer retrieved by the update
// checker is greater than this integer.
#define APP_INTERNAL_REVISION   5

#define APP_NAME                "Gambit"
#define APP_HOMEPAGE            "http://purl.org/net/gambit"
#define APP_FILEVERSION         1, 0, 1, 0
#define APP_FILEVERSION_STR     "1.0.1"
#define APP_FILETYPE            VFT_APP

#endif
