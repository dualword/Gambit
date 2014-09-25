@echo off

if exist CMakeFiles rd /q /s CMakeFiles
if exist CMakeCache.txt del /f CMakeCache.txt
if exist cmake_install.cmake del /f cmake_install.cmake
if exist Makefile del /f Makefile
