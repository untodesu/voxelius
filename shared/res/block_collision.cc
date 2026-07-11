#include "shared/pch.hh"

#include "shared/res/block_collision.hh"

#include "core/res/resource.hh"
#include "core/utils/json.hh"
#include "core/utils/physfs.hh"

static bool parse_aabb(Eigen::AlignedBox3f& aabb, const JSON_Object* object)
{
    if(object == nullptr) {
        return false;
    }

    auto is_valid = true;
    is_valid = is_valid && utils::parse_json(object, "min", aabb.min());
    is_valid = is_valid && utils::parse_json(object, "max", aabb.max());

    if(!is_valid) {
        return false;
    }

    aabb.min() /= 16.0f;
    aabb.max() /= 16.0f;

    return true;
}

static const void* block_collision_load_fn(const char* path, std::uint32_t flags)
{
    std::string source;

    if(!utils::read_file(path, source)) {
        LOG_WARNING("{}: {}", path, utils::physfs_error());
        return nullptr;
    }

    auto json = json_parse_string(source.c_str());
    auto jsonv = json_value_get_object(json);

    BlockCollision collision {};

    auto elements = json_object_get_array(jsonv, "elements");
    auto num_elements = json_array_get_count(elements);
    collision.elements.reserve(num_elements);

    for(std::size_t i = 0; i < num_elements; ++i) {
        if(auto element = json_array_get_object(elements, i)) {
            Eigen::AlignedBox3f aabb {};

            if(!parse_aabb(aabb, element)) {
                LOG_WARNING("{}: invalid element at index {}", path, i);
                json_value_free(json);
                return nullptr;
            }

            collision.bounds.extend(aabb);
            collision.elements.push_back(std::move(aabb));
        }
    }

    json_value_free(json);

    return new BlockCollision(std::move(collision));
}

static void block_collision_free_fn(const void* resource)
{
    delete static_cast<const BlockCollision*>(resource);
}

void BlockCollision::register_resource(void)
{
    res::register_loader<BlockCollision>(&block_collision_load_fn, &block_collision_free_fn);
}
