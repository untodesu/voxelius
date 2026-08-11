#ifndef B5D57737_1608_4D9B_ABD2_70B8358CA53B
#define B5D57737_1608_4D9B_ABD2_70B8358CA53B

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <any>
#include <array>
#include <chrono>
#include <compare>
#include <concepts>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <numbers>
#include <random>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <Eigen/Dense>

#include <BS_thread_pool.hpp>

#include <emhash/hash_set8.hpp>
#include <emhash/hash_table7.hpp>

#include <enet/enet.h>

#include <miniz.h>

#include <parson.h>

#include <physfs.h>

#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>

#include <Tracy.hpp>
#include <TracyC.h>

#include <uulog.hh>

namespace vx::detail
{
template<typename T>
concept hash_string_key = std::same_as<T, std::string>;
template<typename T>
using hash_for = std::conditional_t<hash_string_key<T>, std::hash<std::string_view>, std::hash<T>>;
template<typename T>
using cmp_for = std::conditional_t<hash_string_key<T>, std::equal_to<>, std::equal_to<T>>;
} // namespace vx::detail

namespace vx
{
template<typename Key, typename Value>
using hash_map = emhash7::HashMap<Key, Value, vx::detail::hash_for<Key>, vx::detail::cmp_for<Key>>;
template<typename Key>
using hash_set = emhash8::HashSet<Key, vx::detail::hash_for<Key>, vx::detail::cmp_for<Key>>;
} // namespace vx

#endif /* B5D57737_1608_4D9B_ABD2_70B8358CA53B */
