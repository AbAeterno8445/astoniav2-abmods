@echo off
echo Compiling Source
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c dd.c -o dd.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c engine.c -o engine.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c main.c -o main.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c inter.c -o inter.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c socket.c -o socket.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c sound.c -o sound.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c conv.c -o conv.obj
E:\bcc\bin\bcc32.exe -IE:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c options.c -o options.obj
echo Compiling RES
E:\bcc\bin\brcc32 merc.rc
echo Linking
E:\bcc\bin\ilink32 /aa -LE:\bcc\lib -Llpng -Lzlib -LE:\bcc\lib\psdk E:\bcc\lib\c0w32.obj dd.obj engine.obj main.obj inter.obj socket.obj sound.obj conv.obj options.obj,merc.exe,,cw32mt.lib import32.lib zlib.lib libpng.lib ddraw.lib dsound.lib,merc.def,merc.res
echo Removing .objs
del *.obj
echo Finished
pause