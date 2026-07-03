#ifndef C8465E9C_1D37_4AA0_9BDB_2E2620C6E3CD
#define C8465E9C_1D37_4AA0_9BDB_2E2620C6E3CD

using chunk_pos = Eigen::Vector<std::int32_t, 3>;
using block_pos = Eigen::Vector<std::int64_t, 3>;
using local_pos = Eigen::Vector<std::int32_t, 3>;

using chunk_pos_xz = Eigen::Vector<std::int32_t, 2>;
using block_pos_xz = Eigen::Vector<std::int64_t, 2>;
using local_pos_xz = Eigen::Vector<std::int32_t, 2>;

template<>
struct std::hash<chunk_pos> final {
    constexpr inline std::size_t operator()(const chunk_pos& pos) const noexcept
    {
        std::size_t value = 0;
        value ^= pos.x() * 73856093;
        value ^= pos.y() * 19349663;
        value ^= pos.z() * 83492791;
        return value;
    }
};

template<>
struct std::hash<chunk_pos_xz> final {
    constexpr inline std::size_t operator()(const chunk_pos_xz& pos) const noexcept
    {
        std::size_t value = 0;
        value ^= pos.x() * 73856093;
        value ^= pos.y() * 19349663;
        return value;
    }
};

#endif /* C8465E9C_1D37_4AA0_9BDB_2E2620C6E3CD */
