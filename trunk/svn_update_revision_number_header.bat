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

cd /d "%~dp0"

set rev=

for /f "tokens=1,*" %%i in ('svn info') do (
    if "%%i" == "Revision:" (
        set rev=%%j
        goto :cont
    )
)
:cont

if not "%rev%" == "" goto :cont
echo SVN revision could not be extracted from the output of 'svn info'.>&2
goto :exit
:cont

set file=src\svn_revision_number.h

rem Create an empty file and check for errors, so we know whether it's safe to proceed. Avoids
rem multiple errors to be printed (due to failing `echo >> "%file%"` commands).
del "%file%" >nul 2>&1
if not exist "%file%" goto :cont
echo Failed removing existing file '%file%'.>&2
goto :exit
:cont
echo. 1>nul 2>"%file%"
if exist "%file%" goto :cont
echo Failed creating file '%file%'.>&2
goto :exit
:cont

echo #ifndef SVN_REVISION_NUMBER_H>>"%file%"
echo #define SVN_REVISION_NUMBER_H>>"%file%"
echo.>>"%file%"
echo #define SVN_REVISION_NUMBER %rev% >>"%file%"
echo #define SVN_REVISION_NUMBER_STRING "%rev%">>"%file%"
echo.>>"%file%"
echo #endif>>"%file%"

set exit_with_err=0

:exit
if not "%exit_with_err%" == "0" ( \ 2>nul ) else ( cd . )
