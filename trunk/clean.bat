rmdir /s /q bin
rmdir /s /q proj\cb\obj

mkdir bin

copy res\catacomb\*.cat bin
copy ext\SDL-1.2.15\lib\x86\SDL.dll bin

echo done...