# Source Hierarchy

> **NOTE:** this file is mainly here to discourage me from shuffing sources _again_ because I _HATE_ writing documentation and if I shuffle sources most of documentation will have to be rewritten at some point  

## In-tree directories

|Directory|Description|  
|----|----|  
|`client`|Home for the client executable sources|  
|`core`|Core library. Defines stuff that might be useful without depending on anything block-related|  
|`data`|Stuff that is packed in with game executables in releases, essentially the base game content. Contains at least `builtin` mod but might contain more stuff in future if people contribute|  
|`deps`|Third-party dependencies|  
|`docs`|Website sources, including documentation and a devlog|  
|`scripts`|Scripts utilized by the build system|  
|`server`|Home for the server executable sources|  
|`shared`|Shared game library. Defines the bulk of game logic, networking and whatever stuff both client and server need|  

## Ignored directories

These are emitted by running the game or trying to build it using scripts. These directories are placed inside the `.gitignore` file and are not tracked by Git  

|Directory|Description|  
|----|----|  
|`build`|Root build directory|  
|`dist`|CPack emits distributions here|  
|`mods`|Created by running the game inside the source tree, usually empty but can be used to place mods there for debugging|  

## Directory tree

Made by running `tree.exe` on the repository root  

```
C:\PATH\TO\REPOSITORY
├───.vscode
├───build
├───client
│   ├───res
│   └───utils
├───core
│   ├───config
│   ├───res
│   └───utils
├───data
│   └───builtin
│       ├───collisions
│       │   └───block
│       ├───fonts
│       ├───lang
│       ├───misc
│       ├───models
│       │   └───block
│       ├───scripts
│       │   └───blocks
│       ├───shaders
│       └───textures
│           ├───block
│           └───gui
├───deps
│   ├───eigen
│   ├───emhash
│   ├───enet
│   ├───entt
│   ├───fastnoiselite
│   ├───glad
│   ├───imgui
│   ├───lua
│   ├───parson
│   ├───physfs
│   ├───SDL3
│   ├───stb
│   ├───thread_pool
│   └───uulog
├───docs
│   ├───about
│   ├───dev
│   │   ├───engine
│   │   └───modding
│   └───stylesheets
├───mods
├───scripts
├───server
└───shared
    ├───res
    ├───scripting
    └───utils
```
