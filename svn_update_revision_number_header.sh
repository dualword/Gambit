#!/bin/sh
#
# Written by Jelle Geerts (jellegeerts@gmail.com).
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software.
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

cd "`dirname "$0"`"

rev=`svn info | grep '^Revision: ' | cut -d ' ' -f 2`

if [ "$rev" = '' ]; then
    echo "SVN revision could not be extracted from the output of 'svn info'." >&2
    exit 1
fi

header="\
#ifndef SVN_REVISION_NUMBER_H
#define SVN_REVISION_NUMBER_H

#define SVN_REVISION_NUMBER $rev
#define SVN_REVISION_NUMBER_STRING \"$rev\"

#endif"

echo "$header" > src/svn_revision_number.h
