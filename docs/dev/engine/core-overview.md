# Core Overview

The `core.lib` is a static library used by `client`, `server`, and `shared`. It holds engine internals that do not depend on blocks, worlds, or game rules: process startup, virtual filesystem paths, and a few reusable subsystems.

## What lives here

The parts worth treating as architecture:

- [Configuration](core-config.md): engine configuration

- [Resources](core-resources.md): resource management

- [Threading](core-threading.md): background tasks

- [Buffers](core-buffers.md): binary de-/serialization
