cls
@echo off
gcc *.c -I E:/gcclibs/raylib/include -L E:/gcclibs/raylib/lib ^
-lraylib -lopengl32 -lgdi32 -lwinmm -o game.exe
echo.
echo Build!
.\game
