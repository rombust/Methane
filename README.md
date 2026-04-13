# Methane
This is a conversion of the Commorore Amiga, Super Methane Brothers game.

Build Linux
```bash
cmake -S . -B build
cmake --build build
build/methane
```


Prerequisites Debian/Ubuntu:
```bash
sudo apt install cmake g++ libxrender-dev libasound2-dev libxinerama-dev libvulkan-dev
```

Build Windows Visual Studio.

Using Visual Studio Developer Command Prompt to create the solution in the build folder.
```
cmake -S  . -B build -G "Visual Studio 17 2022"
```


Build Linux - Debug
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
build/methane
```
vulkan validation layers are required for the Debug version
```bash
sudo apt install vulkan-validationlayers
```


Packaging Notes:
The introduction animation is 455 KB. It maybe preferable to exclude this. If the "resources/*.anm" files are not found, and animation will automatically be disabled.


Quick Instructions

Press the CTRL key to start the game.
Use the cursor keys to move around the screen.
Tap the CTRL key to fire gas from the gun.
Hold the CTRL key to suck a trapped baddie into the gun.
Release the CTRL key to throw the trapped baddie from the gun.
Throw baddies at the wall to destroy them.

Licence Note:
This is a conversion of the Commodore Amiga game.

Permission has been given by Apache Software Ltd to release this conversion of the game as GPL V2


