# Sokol Dirt Jam

Rewrite of my original ThreeJs Entry for [Acerola's Dirt Jam](https://itch.io/jam/acerola-dirt-jam) with `C and Sokol`. This repo is based on `floooh's` awesome `cimgui-sokol-starterkit` which can be found [here](https://github.com/floooh/cimgui-sokol-starterkit).

## Build:

```bash
> mkdir build
> cd build

> cmake ..
> cmake --build .
```

To build a Release version on Linux and Mac:

```bash
> cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
> cmake --build .
```

To build a Release version on Windows with the VisualStudio toolchain:

```bash
> cmake ..
> cmake --build . --config MinSizeRel
```

To build in w64devkit console
```
> cmake -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=make ..
> cmake --build .
```
NOTE: on Linux you'll also need to install the 'usual' dev-packages needed for X11+GL development.

## Run:

On Linux and macOS:
```bash
> ./sokol-dirt-jam
```

On Windows with the Visual Studio toolchain the exe is in a subdirectory:
```bash
> Debug\sokol-dirt-jam.exe
> MinSizeRel\sokol-dirt-jam.exe
```

## Build and Run WASM/HTML version via Emscripten (Linux, macOS)

Setup the emscripten SDK as described here:

https://emscripten.org/docs/getting_started/downloads.html#installation-instructions

Don't forget to run ```source ./emsdk_env.sh``` after activating the SDK.

And then in the ```sokol-dirt-jam``` directory:

```
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build .
```

To run the compilation result in the system web browser:

```
> emrun sokol-dirt-jam.html
```

## IDE Support

### Visual Studio (Windows)

On Windows, cmake will automatically create a Visual Studio solution file in
the build directory, which can be opened (inside the build directory) with:

```bash
> cmake --open .
```

### Xcode (macOS)

Replace ```cmake ..``` with ```cmake -GXcode ..``` and open the generated
Xcode project:

```bash
> cmake -GXcode ..
> cmake --open .
```

### Visual Studio Code (Windows, Linux, macOS)

Use the [MS C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
together with the [MS CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
and start Visual Studio code from the project's root directory. The CMake
extension will detect the CMakeLists.txt file and take over from there.

## Acknowledgments
- [cimgui-sokol-starterkit](https://github.com/floooh/cimgui-sokol-starterkit)
