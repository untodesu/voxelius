#include "shared/pch.hh"

#include "shared/res/feature.hh"

#include "core/res/resource.hh"
#include "core/utils/json.hh"
#include "core/utils/physfs.hh"

#include "shared/world/block_registry.hh"

constexpr static std::array ANCHOR_MAPPING = {
    std::make_pair(std::string_view("surface"), static_cast<int>(feature_anchor::SURFACE)),
    std::make_pair(std::string_view("floor"), static_cast<int>(feature_anchor::FLOOR)),
    std::make_pair(std::string_view("ceiling"), static_cast<int>(feature_anchor::CEILING)),
};

constexpr static std::array OVERWRITE_MASK_MAPPING = {
    std::make_pair(std::string_view("gas"), std::uint32_t(BLOCK_TAG_GAS)),
    std::make_pair(std::string_view("rock"), std::uint32_t(BLOCK_TAG_ROCK)),
    std::make_pair(std::string_view("soil"), std::uint32_t(BLOCK_TAG_SOIL)),
    std::make_pair(std::string_view("turf"), std::uint32_t(BLOCK_TAG_TURF)),
    std::make_pair(std::string_view("foil"), std::uint32_t(BLOCK_TAG_FOIL)),
    std::make_pair(std::string_view("wood"), std::uint32_t(BLOCK_TAG_WOOD)),
};

static std::optional<std::vector<block_id_type>> parse_palette(const JSON_Object* json)
{
    auto array = json_object_get_array(json, "palette");
    auto count = json_array_get_count(array);

    if(array == nullptr || count == 0) {
        return std::nullopt;
    }

    std::vector<block_id_type> palette;
    palette.reserve(count);

    for(std::size_t i = 0; i < count; ++i) {
        auto raw = json_array_get_string(array, i);

        if(raw == nullptr) {
            return std::nullopt;
        }

        auto id = Identifier::from_string(raw);
        auto block_id = block_registry::find(id);

        if(block_id == BLOCK_ID_NULL) {
            return std::nullopt;
        }

        palette.emplace_back(block_id);
    }

    return palette;
}

static std::optional<Feature_Part> parse_part(const JSON_Object* part, std::span<const block_id_type> palette)
{
    auto overwrite = utils::parse_bitmask<std::uint32_t>(part, "overwrite", OVERWRITE_MASK_MAPPING);
    auto offset = utils::parse_vector<int, 3>(part, "offset");
    auto index = utils::parse_arithmetic<std::size_t>(part, "block");

    auto valid = true;
    valid = valid && offset.has_value();
    valid = valid && index.has_value() && index.value() < palette.size();

    if(valid) {
        Feature_Part part {};
        part.overwrite_mask = static_cast<block_tag_bit>(overwrite.value_or(0));
        part.block_id = palette[index.value()];
        part.offset = offset.value();
        return part;
    }

    return std::nullopt;
}

static std::optional<std::vector<Feature_Part>> parse_parts(const JSON_Object* json, std::span<const block_id_type> palette)
{
    auto array = json_object_get_array(json, "parts");
    auto count = json_array_get_count(array);

    if(array == nullptr || count == 0) {
        return std::nullopt;
    }

    std::vector<Feature_Part> parts;
    parts.reserve(count);

    for(std::size_t i = 0; i < count; ++i) {
        if(auto part_obj = json_array_get_object(array, i)) {
            auto part = parse_part(part_obj, palette);

            if(!part.has_value()) {
                return std::nullopt;
            }

            parts.push_back(std::move(part.value()));
        }
        else {
            return std::nullopt;
        }
    }

    return parts;
}

static const void* feature_load_fn(const char* path, std::uint32_t flags)
{
    std::string source;

    if(!utils::read_file(path, source)) {
        LOG_WARNING("{}: {}", path, utils::physfs_error());
        return nullptr;
    }

    auto jsonv = json_parse_string(source.c_str());
    auto json = json_value_get_object(jsonv);

    if(jsonv == nullptr) {
        LOG_WARNING("{}: parse error", path);
        return nullptr;
    }

    if(json == nullptr) {
        LOG_WARNING("{}: malformed JSON", path);
        json_value_free(jsonv);
        return nullptr;
    }

    auto palette = parse_palette(json);
    auto palette_data = palette.value_or({});
    auto parts = parse_parts(json, palette_data);
    auto anchor = utils::parse_enum<int>(json, "anchor", ANCHOR_MAPPING);

    json_value_free(jsonv);

    if(parts.has_value()) {
        auto feature = new Feature {};
        feature->anchor = static_cast<feature_anchor>(anchor.value_or(static_cast<int>(feature_anchor::SURFACE)));
        feature->parts = std::move(parts.value());
        return feature;
    }
    else {
        LOG_WARNING("{}: invalid or missing fields", path);
        return nullptr;
    }
}

static void feature_free_fn(const void* resource)
{
    delete static_cast<const Feature*>(resource);
}

void Feature::register_resource(void)
{
    res::register_loader<Feature>(&feature_load_fn, &feature_free_fn);
}
