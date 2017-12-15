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

set "CMAKE_DIR=C:\Dev\cmake-3.10.0-win32-x86"
set "MINGW4_DIR=C:\Qt\Qt5.9.3\Tools\mingw530_32"
set "QTDIR=C:\Qt\Qt5.9.3\5.9.3\mingw53_32"
rem set "CMAKE_PREFIX_PATH=%QTDIR%;%CMAKE_PREFIX_PATH%"
rem set "BOOST_ROOT=Z:\Dev\libraries\boost\1_46_1"
set "PATH=%CMAKE_DIR%\bin;%MINGW4_DIR%\bin;%QTDIR%\bin;%PATH%"

rem The $* token at the end of each doskey macro is replaced by
rem everything following the macro name when entered on the command
rem line, so you can do things like 'br && echo hello', where 'br' is a
rem macro.
doskey c=cmake -G "MinGW Makefiles" -D "CMAKE_BUILD_TYPE=Debug" . $*
doskey cr=cmake -G "MinGW Makefiles" -D "CMAKE_BUILD_TYPE=Release" . $*
doskey b=mingw32-make $*
doskey r=gambitchess $*
doskey br=mingw32-make ^&^& Gambit $*
doskey cl=mingw32-make clean $*
