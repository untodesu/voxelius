#ifndef SHARED_STASIS_HH
#define SHARED_STASIS_HH 1
#pragma once

class Dimension;

// Attached to entities with transform values
// out of bounds in a specific dimension
struct StasisComponent final {
public:
    static void fixed_update(Dimension* dimension);
};

#endif /* SHARED_STASIS_HH */
