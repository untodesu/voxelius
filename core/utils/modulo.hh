#ifndef D0054E57_1D41_4689_AF86_779F44A1E71F
#define D0054E57_1D41_4689_AF86_779F44A1E71F

namespace utils
{
template<std::integral T>
constexpr T mod_signed(T value, T modulus);
} // namespace utils

template<std::integral T>
constexpr T utils::mod_signed(T value, T modulus)
{
    auto result = static_cast<T>(value % modulus);
    if(result < static_cast<T>(0))
        return result + modulus;
    return result;
}

#endif /* D0054E57_1D41_4689_AF86_779F44A1E71F */
