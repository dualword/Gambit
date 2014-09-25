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

# Update only the translation files starting with $appname.
appname=Gambit
nls_dir=nls

opt_no_obsolete=
if [ x$1 = x-no-obsolete ]; then
    opt_no_obsolete=-no-obsolete
elif [ x$1 != x ]; then
    echo "Invalid option: '$1'." >&2
    exit 1
fi

find_lupdate() {
    local path="`which lupdate 2>/dev/null`"
    [ ! "$path" ] && path="`which lupdate-qt4 2>/dev/null`"
    echo "$path"
}

lupdate=`find_lupdate`
if [ ! "$lupdate" ]; then
    echo "Could not find Qt's \`lupdate' tool." >&2
    exit 1
fi

for i in `find "$nls_dir" -maxdepth 1 -name "$appname*.ts"`; do
    "$lupdate" src -ts "$i" $opt_no_obsolete || exit 1
done
