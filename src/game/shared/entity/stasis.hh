#pragma once

class Dimension;

// Attached to entities with transform values
// out of bounds in a specific dimension
struct Stasis final {
public:
    static void fixed_update(Dimension* dimension);
};
