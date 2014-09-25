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

for /f "delims=" %%i in ('dir /s /b moc_*.cxx* 2^>nul') do (
    del "%%i" || (set file=%%i& goto :error)
)

exit /b 0

:error
echo *** ERROR: Could not delete '%file%'.
rem Exit with an error.
\ 2>nul
