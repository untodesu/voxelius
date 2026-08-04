#ifndef B3676B99_D6A9_4E82_9B2C_24D47D79D6FC
#define B3676B99_D6A9_4E82_9B2C_24D47D79D6FC

struct Velocity_Component {
    static void register_component(void);
    static void fixed_update(void);

    Eigen::Vector3f value;
};

struct Velocity_Component_Intr final : public Velocity_Component {};
struct Velocity_Component_Prev final : public Velocity_Component {};

#endif /* B3676B99_D6A9_4E82_9B2C_24D47D79D6FC */
