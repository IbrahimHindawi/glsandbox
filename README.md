# Space Shooter
Attempt at making a game with C and OpenGL.  
C graphics programming with SDL2 & OpenGL:  
Compiles to Win64 MacOS Linux with `compile_commands.json` for clangd LSP support.

https://github.com/IbrahimHindawi/glsandbox/assets/26623065/5d890510-36f5-4b4f-9d63-05279b3fe3b8

### Linux & Darwin Run:
* `mkdir build`
* `cd build`
* `cmake ..`
* `make`
* `./cgfx`

### Win64 Run:
* from `x64 Native Tools Command Prompt for VS 2019`
* [Install & Integrate VCPKG](https://learn.microsoft.com/en-us/vcpkg/examples/installing-and-using-packages)
* Install dependencies through VCPKG.
* `scripts\build.bat -b` to build `.sln` & `compile_commands.json`. (check DCMAKE_TOOLCHAIN_FILE path)
* `scripts\build.bat -cr` to compile & run exe.

### Win64 Toolchain:
* [Visual Studio 2022 & MSBuild](https://visualstudio.microsoft.com/downloads/)
* [Clang Power Tools VS Extension](https://marketplace.visualstudio.com/items?itemName=caphyon.ClangPowerTools)
* [vcpkg](https://github.com/Microsoft/vcpkg/)  
* [CMake](https://cmake.org/)

### Dependencies:
* [SDL2](https://www.libsdl.org/)  
* [CGLM](https://github.com/recp/cglm)  

### Vendored:
* [GLAD](https://glad.dav1d.de/)  
* [stb_image](https://github.com/nothings/stb)  
* [Clang Power Tools script](https://github.com/Caphyon/clang-power-tools/tree/master/ClangPowerTools/ClangPowerTools/Tooling/v1)
