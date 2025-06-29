#ifndef SHARED_GRAVITY_HH
#define SHARED_GRAVITY_HH 1
#pragma once

class Dimension;

struct GravityComponent final {
public:
    static void fixed_update(Dimension* dimension);
};

#endif /* SHARED_GRAVITY_HH */
