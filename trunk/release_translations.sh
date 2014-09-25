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

nls_dir=nls
nls_release_dir=data/nls

find_lrelease() {
    local path="`which lrelease 2>/dev/null`"
    [ ! "$path" ] && path="`which lrelease-qt4 2>/dev/null`"
    echo "$path"
}

lrelease=`find_lrelease`
if [ ! "$lrelease" ]; then
    echo "Could not find Qt's \`lrelease' tool." >&2
    exit 1
fi

for ts in `find "$nls_dir" -maxdepth 1 -name "*.ts"`; do
    qm="`basename $ts .ts`.qm"
    "$lrelease" "$ts" -qm "$nls_release_dir/$qm" || exit 1
done
