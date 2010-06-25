@echo off
set VC7=%ProgramFiles%\Microsoft Visual Studio .NET
set VC71=%ProgramFiles%\Microsoft Visual Studio .NET 2003
set VC8=%ProgramFiles%\Microsoft Visual Studio 8
set VC9=%ProgramFiles%\Microsoft Visual Studio 9.0
set Tools=Common7\Tools
set path=%VC9%\%Tools%;%VC8%\%Tools%;%VC71%\%Tools%;%VC7%\%Tools%;%path%
call vsvars32.bat

cd scintilla\win32
nmake -f scintilla.mak
if errorlevel 1 exit

cd ..\..
cd scite\win32
nmake -f scite.mak

cd ..\..
DEL /S /Q *.a *.aps *.bsc *.dsw *.idb *.ilc *.ild *.ilf *.ilk *.ils *.lib *.map *.ncb *.obj *.o *.opt *.pdb *.plg *.res *.sbr *.tds *.exp > NUL 2<&1
DEL /Q scintilla\bin\*.dll > NUL 2<&1
DEL /Q scite\bin\*.properties > NUL 2<&1
