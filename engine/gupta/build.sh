#!/bin/sh
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

fail() {
    exit 1
}

config=

if [ "$1" = release ]; then
    echo Compiling release build ...
    config="$config -O3 -s -D NDEBUG"
    shift
else
    echo Compiling debug build ...
    config="$config -O -g -D DEBUG"
    [ "$1" = debug ] && shift
fi

CFLAGS="$CFLAGS
-Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-align
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls
-Wnested-externs -Wstrict-prototypes -Wformat=2 -Wundef
-pedantic -pedantic-errors -Wa,--fatal-warnings
"

cc $CFLAGS $config \
-I src \
src/enforce.c \
src/log.c \
src/uassert.c \
src/cecp/cecp.c \
src/cecp/signal.c \
src/cecp/stdin_io.c \
src/engine/board.c \
src/engine/delta_movement_info.c \
src/engine/eval.c \
src/engine/fen.c \
src/engine/gupta.c \
src/engine/move.c \
src/engine/move_deltas.c \
src/engine/piece.c \
src/engine/rules.c \
src/engine/search.c \
-o gupta || fail

echo OK.
