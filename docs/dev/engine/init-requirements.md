# Requirements

## Network access

- In general, the source code is designed around bringing its own dependencies in source form when possible. The only case when it _might_ need network access is when you're building for Windows systems and build system can't seem to locate a working SDL3 installation, then it downloads its own  

- Other than that, you only need Internet to clone the repository  

## Software

- Any C++20 compiler will suffice. Although it is worth noting that you might have some issues building for a system that doesn't provide fixed-width integers (see cppreference for `cstdint`) in the stadnard library  

- A C99 compiler is required; a good amount of dependencies are written in C and will need to be compiled. I am not providing any guarantees that building them with a C++ compiler is going to work as expected  

- CMake of version 3.15 is required, as long as a working backend (Visual Studio, GNU make, literally anything CMake can generate build files for) is required  

- Python 3.x is required for running internal scripts  

- For actual development, you'd need a text editor of sorts. I personally use Visual Studio Code, although there's quite a foster of other editors you can choose from, pick whichever horrible Vim fork that tickes your fancy  

## Brains

- You should probably know what modern features C++20 offers and probably resort to using older stuff with consideration. Genuinely, a lot of stuff that was made by abusing undefined behaviour, stuff like bit-casting `float` to `std::uint32_t`, is now a part of the standard; the example I provided now uses `std::bit_cast` for this, much simpler than whatever horrible union hack or `reinterpret_cast` sorcery was in its place before  

- Maybe perchance possibly perhaps probably try testing your code. I like writing things blind and praying they will work as intended but you can never be sure  
