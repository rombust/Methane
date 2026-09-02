# Super Methane Brothers

A conversion of the Commodore Amiga game *Super Methane Brothers*, running on
Linux, Windows and Android.

Puff and Blow each carry a methane gas gun. Trap a bad guy in a cloud of gas,
suck the cloud into the gun, then throw it at a wall to destroy him.

---

## Building

### Linux

```bash
cmake -S . -B build
cmake --build build
build/methane
```

Prerequisites on Debian and Ubuntu:

```bash
sudo apt install cmake g++ libxrender-dev libasound2-dev libxinerama-dev libvulkan-dev
```

A debug build additionally needs the Vulkan validation layers:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
sudo apt install vulkan-validationlayers
```

### Windows

From a Visual Studio Developer Command Prompt, with the
[Vulkan SDK](https://vulkan.lunarg.com/sdk/home) installed:

```
cmake -S . -B build -G "Visual Studio 17 2022"
```

or

```
cmake -S . -B build -G "Visual Studio 18 2026"
```

Then open the solution in the `build` folder.

### Android

Open the `android` folder in Android Studio and let Gradle sync.

| | |
| --- | --- |
| Minimum | Android 8.0, API 26 |
| Built for | `arm64-v8a`, `x86_64` |
| Renderer | Vulkan 1.1 |

`x86_64` is there for the emulator. Dropping it from `abiFilters` in
`android/app/build.gradle.kts` roughly halves the build time when working on a
physical device.

Debug builds look for the Vulkan validation layer in
`android/app/src/debug/jniLibs/<abi>/libVkLayer_khronos_validation.so`.
download it from the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) and 
place a copy in each ABI folder you build for. 
A missing ABI directory is skipped silently, so it is worth confirming the
APK really contains the layer if you expect validation to run.

---

## Playing

| | |
| --- | --- |
| Move | Left and right |
| Jump | Up - hold it to jump higher |
| Descend | Down, once you have the wings |
| Shoot | Tap fire |
| Suck | Hold fire |
| Throw | Release fire |

Player one uses the cursor keys and CTRL; player two uses W, A, S, D and SHIFT.
A gamepad or joystick can be chosen for either player from the Options screen.

On a touch screen the game draws its own D-pad and fire buttons, with a back
button in the corner that returns to the title screen. There are options for a
left-handed layout, and for playing in portrait or either landscape
orientation.

Settings and high scores are remembered between runs. On Android they are kept
in the application's private directory, and elsewhere under the usual per-user
configuration path.

Full instructions are on the Instructions screen in the game.

---

## Licence

This conversion is free software, released under the GNU General Public License
version 2 or later. See [copying.txt](copying.txt) for the full text.

**The original Amiga version of Super Methane Brothers remains a commercial
game, and its licence has not changed.** Permission was given by Apache Software
Ltd to release *this conversion* under the GPL. Only the source code in this
repository is covered by it.

Portions are derived from ClanLib, which is distributed under a zlib style licence.

See [authors.txt](authors.txt) for credits, including the original Amiga
development team, and [history.txt](history.txt) for the version history.

<https://github.com/rombust/Methane>
