#pragma once

template<typename T>
using resource_ptr = std::shared_ptr<const T>;

constexpr std::uint32_t RESOURCE_CACHE = 0x00000001U; ///< Cache the resource after loading
constexpr std::uint32_t RESOURCE_USER = 0xFFFFFF00U;  ///< User-defined flags for custom behavior

using ResourceLoadFunc = const void* (*)(const char* name, std::uint32_t flags);
using ResourceFreeFunc = void (*)(const void* resource);

namespace resource::detail
{
void register_loader(const std::type_info& type, ResourceLoadFunc load_func, ResourceFreeFunc free_func);
resource_ptr<void> load_resource(const std::type_info& type, std::string_view name, std::uint32_t flags);
resource_ptr<void> find_resource(const std::type_info& type, std::string_view name);
} // namespace resource::detail

namespace resource
{
template<typename T>
void register_loader(ResourceLoadFunc load_func, ResourceFreeFunc free_func);
template<typename T>
resource_ptr<T> load(std::string_view name, std::uint32_t flags = 0U);
template<typename T>
resource_ptr<T> find(std::string_view name);
} // namespace resource

namespace resource
{
void hard_cleanup(void);
void soft_cleanup(void);
} // namespace resource

template<typename T>
void resource::register_loader(ResourceLoadFunc load_func, ResourceFreeFunc free_func)
{
    resource::detail::register_loader(typeid(T), load_func, free_func);
}

template<typename T>
resource_ptr<T> resource::load(std::string_view name, std::uint32_t flags)
{
    auto result = resource::detail::load_resource(typeid(T), name, flags);
    return std::reinterpret_pointer_cast<const T>(result);
}

template<typename T>
resource_ptr<T> resource::find(std::string_view name)
{
    auto result = resource::detail::find_resource(typeid(T), name);
    return std::reinterpret_pointer_cast<const T>(result);
}
