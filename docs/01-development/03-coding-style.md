# Development: coding style

## Clang-format

All C++ code is automatically formatted using `clang-format`, the configuration of which is placed in the repository root. If your editor supports format-on-save you're in the clear, otherwise format changes before committing.  

Dependencies are excluded from automatic formatting since they practically never use the same styling as Voxelius.  

Notable settings:  

|Settings|Value|  
|----|----|  
|Column limit|140|  
|Indentation|4 spaces|  
|Line endings|CRLF|  
|Braces|Custom, see below|  
|Pointer alignment|Left|  
|Reference alignment|Left|  

> **NOTE:** if a formatting result looks wrong, it's probably not the best idea to try and edit `.clang-format`. Instead, maybe possibly probably perhaps try to split your code into chunks so they are better formatted, please... 👉👈  

## Naming

|Kind|Style|Example|  
|----|----|----|  
|Files|`snake_case`|`shared/block_storage.cc`|  
|Namespaces|`snake_case`|`namespace block_storage`|  
|Classes and structures|`PascalCase`|`class ReadBuffer`|  
|Functions and methods|`snake_case`|`block_registry::name_of(...)`|  
|Public fields and members|`snake_case`|`BlockDefinition::model_offset`|  
|Private fields and members|`m_snake_case`|`Chunk::m_blocks`|  
|Constants|`UPPER_SNAKE_CASE`|`BASE_WIDTH`|  
|Type aliases|Inherited|...|  

## Brace newlines

- Functions, methods and namespaces open their brace on a new line  

- Classes, structs, enums, unions, control statements and lambdas keep the brace on the same line  

- Stuff like `else` and `catch` go on a new line after the closing brace. Honestly I don't understand why having two opposite braces and a keyword on the same line is remotely considered readable  

Example:  

```cpp
namespace vx
{
void setup(void);
void teardown(void);
} // namespace vx

void vx::setup(void)
{
    // empty
}

void vx::teardown(void)
{
    Type name {};
    name.do_something();

    if(name.valid()) {
        // ...
    }
    else {
        // ...
    }
}
```

## Styling considerations

- Classes and structures are to be marked `final` unless they're explicitly designed as a base  

- Functions and methods that take zero arguments spell it out in C-style - a `void` keyword is placed inside parentheses  

- Compile-time (`constexpr`) function bodies are to be placed inside the header they're declared in, just after all the declarations  

- C++ headers have the `.hh` extension and C++ sources have the `.cc` extension  

- Profanity inside source code is allowed because I own the code and I can comment whatever the fuck I please inside of it  

- Use the standard `assert` for debug-time checks and `vx::throw_if_xxxx` for cases the game should crash if something goes wrong  

## Include guards

- Every header uses an ifndef-block with a random GUID-style token with dashes replaced with underscores  

- No `#pragma once`  

- The ifndef block terminates with a block comment with the same GUID  

Example:  

```cpp
#ifndef B843EAA9_60C3_4C2B_8036_DF1026035AA8
#define B843EAA9_60C3_4C2B_8036_DF1026035AA8

// ...

#endif /* B843EAA9_60C3_4C2B_8036_DF1026035AA8 */
```

> **NOTE:** this is practically the default behaviour of the [C/C++ Include Guard](https://marketplace.visualstudio.com/items?itemName=akiramiyakoda.cppincludeguard) VSCode extension  

## Namespaces

-  Namespaces are organized by module and are frequently reopened in the same header per logical group of declarations. Internal/private stuff goes into a `detail` sub-namespace.  

-  Every closing brace gets a `// namespace name` comment enforced by clang-format.  

Example:  

```cpp
namespace vx::detail
{
class Exception {
  // ...
};
} // namespace vx::detail

namespace vx
{
using runtime_error = detail::TaggedException<struct runtime_error_tag>;
} // namespace vx
```

## Include grouping

- Each engine module has its own precompiled header, it is included first for every compiled source  

- Includes are sorted in dependency order (eg. `core`, then `shared`, then `client`)  

- Includes within a group are sorted alphabetically via clang-format  

Example:

```cpp
#include "client/pch.hh"

#include "client/game.hh"

#include "core/buffer.hh"
#include "core/exception.hh"
#include "core/identifier.hh"
#include "core/res/resource.hh"

#include "shared/block_registry.hh"
#include "shared/block_storage.hh"
#include "shared/coord.hh"
#include "shared/mod_loader.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/lua.hh"
#include "shared/world.hh"

#include "client/globals.hh"
#include "client/res/texture2D.hh"
```

## Comments

- Plain C++ comments (except for include guards)  

- Comments are to be used sparingly and only to explain non-obvious stuff, maybe express a frustration with something, just read what exists right now, show-dont-tell  

- Empty function/method bodies are to be marked with an `// empty` comment  

Example 1:  

```cpp
explicit Exception(std::string_view what, std::source_location location = std::source_location::current())
{
  // empty
}
```

Example 2:  

```cpp
// NOTE: SDL seems to defer handle release until the GPU
// is truly done with the object, so it's probably safe
// to even release the old handle mid-frame
reset();
```

## See also

- [Clang-format is good enough](https://untode.su/posts/2026-02-16-clang-format/)  
