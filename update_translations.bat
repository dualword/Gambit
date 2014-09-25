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

rem Update only the translation files starting with %appname%.
set appname=Gambit

set nls_dir=nls

set opt_no_obsolete=
if x%1 == x-no-obsolete (
    set opt_no_obsolete=-no-obsolete
) else if not x%1 == x (
    echo Invalid option: '%1'.>&2
    goto :exit
)

for /f "delims=" %%i in ('dir /b "%nls_dir%\%appname%*.ts"') do (
    lupdate src -ts "%nls_dir%\%%i" %opt_no_obsolete% || goto :exit
)

set exit_with_err=0

:exit
if not "%exit_with_err%" == "0" ( \ 2>nul ) else ( cd . )
