# Requirements

## Network access

The source tree vendors most dependencies when it can. Network access is rarely required.

The main exception is a Windows build that cannot find a working SDL3 install. In that case the build may download SDL3.

You also need network access to clone the repository.

## Software

- A C++20 compiler. You may hit issues on platforms that lack fixed-width integers in the standard library (see cppreference for `cstdint`).
- A C99 compiler. Many dependencies are C and must be compiled as C. Building them with a C++ compiler is not guaranteed to work.
- CMake 3.15 or newer, plus a working generator backend (Visual Studio, GNU make, or any other backend CMake supports).
- Python 3.x for internal scripts.
- A text editor. Visual Studio Code works. Use whatever editor you prefer.

## Brains

Know modern C++20 features and prefer them over older patterns when they fit. Many old undefined-behaviour tricks are now standard. For example, bit-casting `float` to `std::uint32_t` uses `std::bit_cast` instead of union hacks or `reinterpret_cast`.

Test your code. Blind coding and hope are not a substitute for checks.
