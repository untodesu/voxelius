# Overview

Voxelius _the game_ builds on Voxelius _the engine_. The engine exposes Lua hooks. Mods use these hooks to define blocks and other content that extends core gameplay.

## Namespaced identifiers

The engine uses _namespaced identifiers_, similar to Minecraft. A namespaced identifier has a namespace and an identifier:

```
[<namespace>:]<identifier>
```

The namespace is optional.

- In a mod script, a missing namespace defaults to the namespace of that mod.
- Elsewhere, a missing namespace defaults to `builtin` (the base game "mod" ID).

Valid characters in both namespace and identifier:

- Latin letters: `A-Z` and `a-z`
- Digits: `0-9`
- Underscore: `_`
- Tilde: `~` (not recommended for general use, see below)

On init, if the engine finds a duplicate registry entry (for example `mymod:myblock` registered twice), it appends `~N` and retries. After 10000 renames, behaviour is undefined.

## Error handling

The mod root script `modname/scripts/init.lua` can catch its own errors. If an error escapes that script and reaches the init `lua_pcall`, the engine marks the mod load as a failure.
