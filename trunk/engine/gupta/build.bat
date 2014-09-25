@echo off
rem Written by Jelle Geerts (jellegeerts@gmail.com).
rem
rem To the extent possible under law, the author(s) have dedicated all
rem copyright and related and neighboring rights to this software to
rem the public domain worldwide. This software is distributed without
rem any warranty.
rem
rem You should have received a copy of the CC0 Public Domain Dedication
rem along with this software.
rem If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

setlocal
set exit_with_err=1

set config=

if not "%1" == "release" goto :debug
echo Compiling release build ...
set config=%config% -O3 -s -D NDEBUG
shift
goto :cont
:debug
echo Compiling debug build ...
set config=%config% -O -g -D DEBUG
if "%1" == "debug" shift
:cont

set CFLAGS=%CFLAGS% ^
-Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-align ^
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls ^
-Wnested-externs -Wstrict-prototypes -Wformat=2 -Wundef ^
-pedantic -Wa,--fatal-warnings

windres -I src src\resource-win32\rsrc.rc resources.coff || goto :exit

gcc %CFLAGS% %config% ^
-I src ^
src\enforce.c ^
src\log.c ^
src\uassert.c ^
src\cecp\cecp.c ^
src\cecp\signal.c ^
src\cecp\stdin_io.c ^
src\engine\board.c ^
src\engine\delta_movement_info.c ^
src\engine\eval.c ^
src\engine\fen.c ^
src\engine\gupta.c ^
src\engine\move.c ^
src\engine\move_deltas.c ^
src\engine\piece.c ^
src\engine\rules.c ^
src\engine\search.c ^
resources.coff ^
-o gupta.exe || goto :exit

set exit_with_err=0
echo OK

:exit
if exist resources.coff del resources.coff
if not "%exit_with_err%" == "0" ( \ 2>nul ) else ( cd . )
