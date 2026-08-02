#ifndef C8465E9C_1D37_4AA0_9BDB_2E2620C6E3CD
#define C8465E9C_1D37_4AA0_9BDB_2E2620C6E3CD

using ChunkPos = Eigen::Vector<std::int32_t, 3>;
using BlockPos = Eigen::Vector<std::int64_t, 3>;
using LocalPos = Eigen::Vector<std::int32_t, 3>;

using ChunkPosXZ = Eigen::Vector<std::int32_t, 2>;
using BlockPosXZ = Eigen::Vector<std::int64_t, 2>;
using LocalPosXZ = Eigen::Vector<std::int32_t, 2>;

using ChunkAlignedBox = Eigen::AlignedBox<ChunkPos::value_type, 3>;
using BlockAlignedBox = Eigen::AlignedBox<BlockPos::value_type, 3>;
using LocalAlignedBox = Eigen::AlignedBox<LocalPos::value_type, 3>;

using ChunkAlignedBoxXZ = Eigen::AlignedBox<ChunkPosXZ::value_type, 2>;
using BlockAlignedBoxXZ = Eigen::AlignedBox<BlockPosXZ::value_type, 2>;
using LocalAlignedBoxXZ = Eigen::AlignedBox<LocalPosXZ::value_type, 2>;

template<>
struct std::hash<ChunkPos> final {
    constexpr inline std::size_t operator()(const ChunkPos& pos) const
    {
        std::size_t value = 0;
        value ^= pos.x() * 73856093;
        value ^= pos.y() * 19349663;
        value ^= pos.z() * 83492791;
        return value;
    }
};

template<>
struct std::hash<ChunkPosXZ> final {
    constexpr inline std::size_t operator()(const ChunkPosXZ& pos) const
    {
        std::size_t value = 0;
        value ^= pos.x() * 73856093;
        value ^= pos.y() * 19349663;
        return value;
    }
};

template<>
struct std::hash<BlockPosXZ> final {
    constexpr inline std::size_t operator()(const BlockPosXZ& pos) const
    {
        std::size_t value = 0;
        value ^= pos.x() * 73856093;
        value ^= pos.y() * 19349663;
        return value;
    }
};

#endif /* C8465E9C_1D37_4AA0_9BDB_2E2620C6E3CD */
