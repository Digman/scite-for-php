@ECHO OFF
SET PATH=C:\MinGW\bin;%ProgramFiles%\CodeBlocks\bin;C:\MinGW\upx;%PATH%

mingw32-make
//if errorlevel 1 exit

mingw32-make clean
upx.exe --best gui.dll