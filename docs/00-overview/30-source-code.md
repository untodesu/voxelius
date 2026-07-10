# Overview: source code

Source code hierarchy has changed a couple of times, though I hope the current state is going to somewhat stick because I already wrote documentation for it...  

## Top-level layout

|Directory|Notes|Description|  
|----|----|----|  
|`build`|ignored|Build directory used by build and packaging scripts|  
|`client`| |Client executable; mainly does rendering, GUI and input processing|  
|`core`| |Core library; defines stuff that might be useful outside of scope of game executables|  
|`data`| |Built-in game data aka the `builtin` quote-unquote mod|  
|`deps`| |Third-party dependencies|  
|`docs`| |Documentation|  
|`mods`|ignored|Created by running the game inside the source tree|  
|`scripts`| |Utility scripts used to invoke CMake and by CMake scripts|  
|`server`| |Server executable; mainly does headless game logic|  
|`shared`| |Shared game library; contains the bulk of game logic and networking|  

## Library: `core`

The bottommost layer. Defines common stuff that one might want to use to interface with the game without needing to simulate actual gameplay. Stuff like configuration, filesystem utilities, maths and resource management lives here  

|Subdirectory|Notes|Description|  
|----|----|----|  
|`config`| |Key-value ini-like configuration|  
|`res`| |Resource management|  
|`utils`| |Small standalone helpers|  

## Library: `shared`

Everything that the actual game needs to share between client and server. Almost all game logic and world stuff goes here, defines block storage formats, region formats, scripting hooks and so on.  

|Subdirectory|Notes|Description|  
|----|----|----|  
|`scripting`| |Libraries for per-mod Lua runtimes|  
|`utils`|Small standalone helpers|  

## Executable: `client`

Contains sources for the client-side game executable. The main purpose of it is to render the world, process the inputs and talk with the server through the network...  

|Subdirectory|Notes|Description|  
|----|----|----|  
|`gpu`| |Thin C++ wrappers for SDL3_GPU primitives|  
|`res`| |Resource management|  

## Executable: `server`

> **TODO:** document this!

## Directory tree

Made by running `tree.exe` on the repository root  

```
C:\PATH\TO\REPOSITORY
├───.vscode
├───build
├───client
│   ├───gpu
│   └───res
├───cmake
├───core
│   ├───config
│   ├───res
│   └───utils
├───data
│   └───builtin
│       ├───models
│       │   ├───bcoll
│       │   └───block
│       ├───scripts
│       │   └───blocks
│       └───textures
├───deps
│   ├───eigen
│   ├───emhash
│   ├───enet
│   ├───entt
│   ├───imgui
│   ├───lua
│   ├───parson
│   ├───physfs
│   ├───SDL3
│   ├───stb
│   ├───thread_pool
│   └───uulog
├───docs
│   ├───00-overview
│   ├───01-development
│   └───02-modding
├───mods
├───scripts
├───server
└───shared
    ├───scripting
    └───utils
```
