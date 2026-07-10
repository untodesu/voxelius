#ifndef D5BD65AB_8EF2_4971_A0F9_86C72E119FB0
#define D5BD65AB_8EF2_4971_A0F9_86C72E119FB0

struct BlockCollision final {
    static void register_resource(void) noexcept;

    std::vector<Eigen::AlignedBox3f> elements;
    Eigen::AlignedBox3f bounds;
};

#endif /* D5BD65AB_8EF2_4971_A0F9_86C72E119FB0 */
