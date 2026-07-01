#ifndef BE3E6E7A_D7CE_4392_BEBB_1A7AB35C0124
#define BE3E6E7A_D7CE_4392_BEBB_1A7AB35C0124

constexpr static std::uint32_t RESFLAG_CACHE = 1 << 0;      ///< Cache the resource after loading
constexpr static std::uint32_t RESFLAG_CUSTOM = 0xFFFFFF00; ///< Mask of custom resource flags/data

namespace res
{
template<typename T>
using handle = std::shared_ptr<const T>;
} // namespace res

namespace res
{
using load_func = const void* (*)(const char* name, std::uint32_t flags);
using free_func = void (*)(const void* resource);
} // namespace res

namespace res::detail
{
void register_loader(const std::type_info& type, load_func load_fn, free_func free_fn);
handle<void> load_resource(const std::type_info& type, std::string_view name, std::uint32_t flags);
handle<void> find_resource(const std::type_info& type, std::string_view name);
} // namespace res::detail

namespace res
{
template<typename T>
void register_loader(load_func load_fn, free_func free_fn);
template<typename T>
handle<T> load(std::string_view name, std::uint32_t flags = 0);
template<typename T>
handle<T> find(std::string_view name);
} // namespace res

namespace res
{
void soft_purge(bool include_cached = false);
void hard_purge(void);
} // namespace res

template<typename T>
void res::register_loader(load_func load_fn, free_func free_fn)
{
    res::detail::register_loader(typeid(T), load_fn, free_fn);
}

template<typename T>
res::handle<T> res::load(std::string_view name, std::uint32_t flags)
{
    auto result = res::detail::load_resource(typeid(T), name, flags);
    return std::reinterpret_pointer_cast<const T>(result);
}

template<typename T>
res::handle<T> res::find(std::string_view name)
{
    auto result = res::detail::find_resource(typeid(T), name);
    return std::reinterpret_pointer_cast<const T>(result);
}

#endif /* BE3E6E7A_D7CE_4392_BEBB_1A7AB35C0124 */
