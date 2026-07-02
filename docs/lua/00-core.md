# Lua API: Core

Base utilities available in every Lua state from the moment it's created — logging and script loading. Everything here lives under the `core` global table.

## Constants

### `core.VERSION`

Specifies game versions. See `core/version.hh` for the C++ equivalent  

|Value|Description|  
|----|----|  
|`core.VERSION_MAJOR`|Major version number|  
|`core.VERSION_MINOR`|Minor version number|  
|`core.VERSION_PATCH`|Patch version number|  
|`core.VERSION_SCM_BRANCH`|SCM branch name|  
|`core.VERSION_SCM_REVISION`|SCM revision aka git commit hash|  
|`core.VERSION_SEMANTIC`|Semantic version string|  
|`core.VERSION`|Full build-system-defined version string|  

## Functions

### `core.log_info(...)`

Print an informational message to the game console/log.

### `core.log_warning(...)`

Print a warning to the game console/log. Used for recoverable issues (e.g. a missing texture falling back to a placeholder, a deprecated field still being read).

### `core.log_error(...)`

Print an error to the game console/log. Used for issues that don't stop loading but leave something broken (e.g. `blocks.add` id conflict, malformed blockmodel).

### `core.log_critical(...)`

Print a critical error to the game console/log. Implies the game cannot continue in a well-defined state; the host application may abort after printing.

### `core.log_debug(...)`

Print a debug message. Only shown when the engine is running with debug logging enabled; a no-op (but still valid to call) otherwise, so mods don't need to guard calls with a debug flag check.

### `core.do_file(path)`

Load and immediately execute another Lua script, in the same Lua state (shares globals with the caller).

**Arguments:**

- `path` is a namespaced virtual filesystem path, eg. `"builtin:blocks/stone.lua"`;  

**Notes:**

- The function doesn't use caching. Calling `do_file` twice with the same path just executes the file two times;  

- A missing or malformed file raises a Lua error; any mod that raises an error is assumed to fail to load. This _does not_ abort the whole game boot unless it's a `builtin` mod;  

## Namespaced identifiers

```
[<namespace>:][identifier]
```

- `namespace` can be omitted, in which case `core.NAMESPACE` is used;  
- `identifier` must be a valid C identifier with an exception of an allowed `~` character that follows the same rules as underscores;  

## Error handling

The host tracks everything a mod registers (block IDs, etc.) while its scripts are running. If an uncaught error propagates out of the mod's top-level script, everything that the mod has registered is rolled back and the mod is marked as failed to load. Any other mods that [depend](01-modinfo.md) on the failed mod are also skipped.  

> **NOTE:** if we talk in terms of `builtin` mod, the game straight up crashes because you can't have the game with broken built-in content  
