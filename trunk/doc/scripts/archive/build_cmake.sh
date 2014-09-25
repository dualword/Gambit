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

CMAKE_VER=2.8.5
CMAKE_DIR=cmake-$CMAKE_VER
CMAKE_PREFIX=/opt/cmake/$CMAKE_VER

if [ ! -d "$CMAKE_DIR" ]; then
  echo "The directory '$CMAKE_DIR' could not be found." >&2
  exit 1
fi

cd "$CMAKE_DIR" || exit 1

if [ x$1 = x--install ]; then
  make install || exit 1
else
  ./bootstrap --prefix="$CMAKE_PREFIX" || exit 1
  make || exit 1
fi
exit 0
