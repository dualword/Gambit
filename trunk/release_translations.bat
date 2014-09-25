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

setlocal enabledelayedexpansion
set exit_with_err=1

cd /d "%~dp0"

set nls_dir=nls
set nls_release_dir=data\nls

for /f "delims=" %%i in ('dir /b "%nls_dir%\*.ts"') do (
    set ts=%%i
    set qm=!ts:.ts=.qm!
    lrelease "%nls_dir%\!ts!" -qm "%nls_release_dir%\!qm!" || goto :exit
)

set exit_with_err=0

:exit
if not "%exit_with_err%" == "0" ( \ 2>nul ) else ( cd . )
