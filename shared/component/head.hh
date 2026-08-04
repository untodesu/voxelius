#ifndef E9E2840F_0C1E_4B74_987A_B592B5FD6A5F
#define E9E2840F_0C1E_4B74_987A_B592B5FD6A5F

struct Head_Component {
    static void register_component(void);

    Eigen::Vector3f angles;
    Eigen::Vector3f offset;
};

struct Head_Component_Intr final : public Head_Component {};
struct Head_Component_Prev final : public Head_Component {};

#endif /* E9E2840F_0C1E_4B74_987A_B592B5FD6A5F */
