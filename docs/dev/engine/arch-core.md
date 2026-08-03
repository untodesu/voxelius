# Core

The core is the foundation of the engine. Client, server, and shared game logic all build on it. The core gives every part of the engine the basic things it needs, whether or not that part renders anything or talks to a network. The core manages file locations, process start and stop, error reporting, and a few small subsystems. Other, more game-specific parts of the engine use these subsystems.

## Process lifecycle

When the engine starts, it resolves three directories: the game data directory, the user data directory, and the mods directory. The user data directory location depends on the platform. The engine uses `%APPDATA%`, the XDG data directory, `$HOME`, or the current directory, in that order, as fallbacks. The engine mounts all three directories into one virtual filesystem. The user directory is writable. The engine searches the mods directory last. All other engine parts, such as configs, resources, and mods, read and write through this virtual filesystem instead of the physical disk. On shutdown, the engine reverses these steps.

A few small pieces support this startup process. The engine parses command-line options once at startup. Other code can query these options by name, for example to pick a thread pool size. The engine also uses one exception convention, `vx::throw_if` and `throw_if_not`, for every failure case. This convention keeps error handling consistent across the codebase. Without it, each subsystem could invent its own method.

Resources and mods use namespaced identifiers, for example `mymod:some/thing`, instead of raw paths. The namespace maps to a mod. The engine reuses this identifier convention for blocks, biomes, sounds, and other moddable items.

## Configuration

Configuration is a generic key-value store, with string keys. Configuration does not belong to one feature. Any part of the engine that needs persistent settings, such as user preferences, server settings, or mod metadata, loads its settings into this same kind of map. Code can bind a typed reference to a key. This keeps reads cheap when the value does not change. The client and the server each keep one global config as the default place for their own settings. A subsystem can also load its own separate, temporary map. Mod metadata does this.

## Resources

Resource management is the engine's one loading and caching path for assets that mods reference. A resource type registers one loader for itself. After that, any code can request a resource of that type by identifier. The engine turns the identifier into a file path and looks it up. The request returns an existing instance if one is already loaded, or triggers a new load and keeps the result for later requests. A request can also ask the engine to hold a permanent reference to the result, so the resource survives even while nothing else is using it.

Purging follows a fixed memory policy; it is not a one-off cleanup call. A soft purge drops only the resources that nothing references anymore, and normally runs after every frame. A hard purge drops every resource regardless of use, and warns about each one still in use. That warning is a sign that something is holding on to memory the engine just freed.

## Threading

The engine pushes heavy, non-interactive work onto a background thread pool instead of running it inline. Examples of this work include world generation and chunk meshing, or any CPU-heavy task that would otherwise stall a frame. The pool size comes from the `threads` command-line option, and defaults to four workers.

The engine splits each piece of work into two parts. Computation runs on a worker thread. Any step that touches shared state, such as uploading a mesh or changing the world, waits until the computation finishes. The main loop then checks, once per frame, which tasks have finished, and runs that second step for each of them on the main thread. This design removes the need for manual locking in most cases. On shutdown, the engine cancels any task that has not finished yet, instead of waiting for it or running its main-thread step.

## Buffers

Buffers are the engine's one shared binary format. Everything else uses buffers to describe bytes sent over the network or bytes saved to disk. The engine defines a small set of base types once: fixed-width integers, floats, strings, and vectors. Anything that needs serialization, such as network packets or saved data, is described as a sequence of these base types. Because the vocabulary is shared, the network protocol and the on-disk formats reuse the same description: each format states which buffer field comes next, and nothing needs to redefine what a byte means.
