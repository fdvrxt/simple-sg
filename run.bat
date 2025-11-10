@echo off
if not exist build mkdir build
cd /d build

rem Configure (VS 2022 x64 — tweak if needed)
cmake -G "Visual Studio 17 2022" -A x64 ..

rem Build (Debug by default)
cmake --build . --config Release
