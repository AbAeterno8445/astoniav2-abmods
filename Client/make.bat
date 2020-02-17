@echo off
SET BINPATH=E:\bcc\bin
SET INCPATH=E:\bcc\include
SET LIBPATH=E:\bcc\lib
echo Compiling Source
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c dd.c -o dd.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c engine.c -o engine.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c main.c -o main.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c inter.c -o inter.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c socket.c -o socket.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c sound.c -o sound.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c conv.c -o conv.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c options.c -o options.obj
echo Compiling RES
%BINPATH%\brcc32 merc.rc
echo Linking
%BINPATH%\ilink32 /aa -L%LIBPATH% -Llpng -Lzlib -L%LIBPATH%\psdk %LIBPATH%\c0w32.obj dd.obj engine.obj main.obj inter.obj socket.obj sound.obj conv.obj options.obj,merc.exe,,cw32mt.lib import32.lib zlib.lib libpng.lib ddraw.lib dsound.lib,merc.def,merc.res
echo Removing .objs
del *.obj
echo Cleaning up...
del merc.ilc
del merc.ild
del merc.ilf
del merc.ils
del merc.map
del merc.RES
del merc.tds
echo Finished
pause