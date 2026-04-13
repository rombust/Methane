# Methane
This is a conversion of the Commorore Amiga, Super Methane Brothers game.

Build Linux
```bash
cmake -S . -B build
cmake --build build
build/methane
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


Notes:
The introduction animation is 455 KB. It maybe preferable to exclude from the build if required.
The music is 17 MB. It maybe preferable to exclude from the build if required.

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


