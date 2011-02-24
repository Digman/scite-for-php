@ECHO OFF
SET PATH=C:\MinGW\bin;%PATH%;

CD scintilla\win32
mingw32-make
CD ..\..
CD scite\win32
mingw32-make
