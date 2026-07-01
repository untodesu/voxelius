#include "game/shared/pch.hh"

#include "game/shared/utils/coord.hh"

#include "core/utils/modulo.hh"

#include "game/shared/constant.hh"

chunk_pos utils::to_chunk(const block_pos& bpos) noexcept
{
    chunk_pos cpos;
    cpos.x() = static_cast<chunk_pos::value_type>(bpos.x() >> constant::CHUNK_SIZE_LOG2);
    cpos.y() = static_cast<chunk_pos::value_type>(bpos.y() >> constant::CHUNK_SIZE_LOG2);
    cpos.z() = static_cast<chunk_pos::value_type>(bpos.z() >> constant::CHUNK_SIZE_LOG2);
    return cpos;
}

local_pos utils::to_local(const block_pos& bpos) noexcept
{
    constexpr auto CHUNK_SIZE = static_cast<block_pos::value_type>(constant::CHUNK_SIZE);
    local_pos lpos;
    lpos.x() = static_cast<local_pos::value_type>(utils::mod_signed<block_pos::value_type>(bpos.x(), CHUNK_SIZE));
    lpos.y() = static_cast<local_pos::value_type>(utils::mod_signed<block_pos::value_type>(bpos.y(), CHUNK_SIZE));
    lpos.z() = static_cast<local_pos::value_type>(utils::mod_signed<block_pos::value_type>(bpos.z(), CHUNK_SIZE));
    return lpos;
}

local_pos utils::to_local(std::size_t index) noexcept
{
    local_pos lpos;
    lpos.x() = static_cast<local_pos::value_type>(index % constant::CHUNK_SIZE);
    lpos.y() = static_cast<local_pos::value_type>((index / constant::CHUNK_SIZE) / constant::CHUNK_SIZE);
    lpos.z() = static_cast<local_pos::value_type>((index / constant::CHUNK_SIZE) % constant::CHUNK_SIZE);
    return lpos;
}

block_pos utils::to_block(const chunk_pos& cpos, const local_pos& lpos) noexcept
{
    block_pos bpos;
    bpos.x() = static_cast<block_pos::value_type>(lpos.x()) + (static_cast<block_pos::value_type>(cpos.x()) << constant::CHUNK_SIZE_LOG2);
    bpos.y() = static_cast<block_pos::value_type>(lpos.y()) + (static_cast<block_pos::value_type>(cpos.y()) << constant::CHUNK_SIZE_LOG2);
    bpos.z() = static_cast<block_pos::value_type>(lpos.z()) + (static_cast<block_pos::value_type>(cpos.z()) << constant::CHUNK_SIZE_LOG2);
    return bpos;
}

std::size_t utils::to_index(const local_pos& lpos) noexcept
{
    return static_cast<std::size_t>((lpos.y() * constant::CHUNK_SIZE + lpos.z()) * constant::CHUNK_SIZE + lpos.x());
}

Eigen::Vector3f utils::to_relative(const chunk_pos& pivot_cpos, const chunk_pos& cpos, const Eigen::Vector3f& fvec) noexcept
{
    Eigen::Vector3f rpos;
    rpos.x() = static_cast<float>((cpos.x() - pivot_cpos.x()) << constant::CHUNK_SIZE_LOG2) + fvec.x();
    rpos.y() = static_cast<float>((cpos.y() - pivot_cpos.y()) << constant::CHUNK_SIZE_LOG2) + fvec.y();
    rpos.z() = static_cast<float>((cpos.z() - pivot_cpos.z()) << constant::CHUNK_SIZE_LOG2) + fvec.z();
    return rpos;
}

Eigen::Vector3f utils::to_relative(const chunk_pos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const chunk_pos& cpos) noexcept
{
    Eigen::Vector3f rpos;
    rpos.x() = static_cast<float>((cpos.x() - pivot_cpos.x()) << constant::CHUNK_SIZE_LOG2) - pivot_fvec.x();
    rpos.y() = static_cast<float>((cpos.y() - pivot_cpos.y()) << constant::CHUNK_SIZE_LOG2) - pivot_fvec.y();
    rpos.z() = static_cast<float>((cpos.z() - pivot_cpos.z()) << constant::CHUNK_SIZE_LOG2) - pivot_fvec.z();
    return rpos;
}

Eigen::Vector3f utils::to_relative(const chunk_pos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const chunk_pos& cpos,
    const Eigen::Vector3f& fvec) noexcept
{
    Eigen::Vector3f rpos;
    rpos.x() = static_cast<float>((cpos.x() - pivot_cpos.x()) << constant::CHUNK_SIZE_LOG2) + (fvec.x() - pivot_fvec.x());
    rpos.y() = static_cast<float>((cpos.y() - pivot_cpos.y()) << constant::CHUNK_SIZE_LOG2) + (fvec.y() - pivot_fvec.y());
    rpos.z() = static_cast<float>((cpos.z() - pivot_cpos.z()) << constant::CHUNK_SIZE_LOG2) + (fvec.z() - pivot_fvec.z());
    return rpos;
}

Eigen::Vector3f utils::to_fvec(const chunk_pos& cpos) noexcept
{
    Eigen::Vector3f rpos;
    rpos.x() = static_cast<float>(cpos.x() << constant::CHUNK_SIZE_LOG2);
    rpos.y() = static_cast<float>(cpos.y() << constant::CHUNK_SIZE_LOG2);
    rpos.z() = static_cast<float>(cpos.z() << constant::CHUNK_SIZE_LOG2);
    return rpos;
}

Eigen::Vector3f utils::to_fvec(const chunk_pos& cpos, const Eigen::Vector3f& offset) noexcept
{
    Eigen::Vector3f rpos;
    rpos.x() = offset.x() + static_cast<float>(cpos.x() << constant::CHUNK_SIZE_LOG2);
    rpos.y() = offset.y() + static_cast<float>(cpos.y() << constant::CHUNK_SIZE_LOG2);
    rpos.z() = offset.z() + static_cast<float>(cpos.z() << constant::CHUNK_SIZE_LOG2);
    return rpos;
}
