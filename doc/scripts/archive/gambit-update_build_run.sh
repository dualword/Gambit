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

GAMBIT_DIR=$HOME/Desktop/gambit

CMAKE_BIN_DIR=/opt/cmake/2.8.5/bin
if [ -d "$CMAKE_BIN_DIR" ]; then
  export PATH=$CMAKE_BIN_DIR:$PATH
  echo "NOTE: Using custom CMake build from '$CMAKE_BIN_DIR'."
fi

# Check for OpenBSD, and set CC and CXX so a newer version of GCC is used.
uname -s | grep -qi openbsd
if [ $? -eq 0 ]; then
  export CC=egcc
  export CXX=eg++
fi

cd "$GAMBIT_DIR" || exit 1
svn up
sh clean_all.sh
. "$PWD/setup_env.sh"
c
br || read ignore
