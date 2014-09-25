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
cd /d "%~dp0"

if exist "%ProgramFiles%\Inno Setup*" goto :cont
if exist "%ProgramFiles(x86)%\Inno Setup*" goto :cont
echo WARNING: Inno Setup wasn't found to be installed.>&2
:cont

echo Enter the Gambit version number.
echo NOTE: This must equal the version shown in the About box.
(set /p ver=Input: )

if not exist Gambit.iss goto :cont
echo ERROR: 'Gambit.iss' already exists.>&2
pause
goto :EOF
:cont

echo #define MyAppVersion "%ver%">Gambit.iss
type Gambit-template.iss >>Gambit.iss

start Gambit.iss
