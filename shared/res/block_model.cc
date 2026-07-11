#include "shared/pch.hh"

#include "shared/res/block_model.hh"

#include "core/res/resource.hh"

#include "core/utils/json.hh"
#include "core/utils/physfs.hh"

constexpr static float ROTATION_STEP = 15.0f;
constexpr static float ROTATION_MIN = -180.0f;
constexpr static float ROTATION_MAX = +180.0f;

static float snap_rotation(float degrees)
{
    auto clamped = std::clamp(degrees, ROTATION_MIN, ROTATION_MAX);

    return std::round(clamped / ROTATION_STEP) * ROTATION_STEP;
}

static bool parse_block_face_name(const char* name, block_face& face)
{
    if(name == nullptr) {
        return false;
    }

    if(0 == std::strcmp(name, "north")) {
        face = BLOCK_FACE_NORTH;
        return true;
    }

    if(0 == std::strcmp(name, "south")) {
        face = BLOCK_FACE_SOUTH;
        return true;
    }

    if(0 == std::strcmp(name, "east")) {
        face = BLOCK_FACE_EAST;
        return true;
    }

    if(0 == std::strcmp(name, "west")) {
        face = BLOCK_FACE_WEST;
        return true;
    }

    if(0 == std::strcmp(name, "top")) {
        face = BLOCK_FACE_TOP;
        return true;
    }

    if(0 == std::strcmp(name, "bottom")) {
        face = BLOCK_FACE_BOTTOM;
        return true;
    }

    return false;
}

static bool parse_texture_slots(BlockModel& model, const JSON_Array* array)
{
    auto count = json_array_get_count(array);

    if(array == nullptr || count == 0) {
        return false;
    }

    model.texture_slots.clear();
    model.texture_slots.reserve(count);

    for(std::size_t i = 0; i < count; ++i) {
        auto value = json_array_get_string(array, i);

        if(value == nullptr) {
            return false;
        }

        model.texture_slots.emplace_back(value);
    }

    return true;
}

static bool parse_face(BlockModel_Face& face, const JSON_Object* object)
{
    if(object == nullptr) {
        return false;
    }

    auto texture = json_object_get_string(object, "texture");

    if(texture == nullptr) {
        return false;
    }

    face.texture_slot = texture;

    if(json_object_has_value(object, "uv")) {
        Eigen::Vector4f uv;

        if(!utils::parse_json(object, "uv", uv)) {
            return false;
        }

        face.uv = uv / 16.0f;
    }

    face.uv_rotation = 0;

    if(json_object_has_value(object, "uv_rotation")) {
        auto raw = static_cast<int>(json_object_get_number(object, "uv_rotation"));

        if(raw != 0 && raw != 90 && raw != 180 && raw != 270) {
            return false;
        }

        face.uv_rotation = static_cast<unsigned>(raw);
    }

    if(json_object_has_value(object, "cullface")) {
        auto name = json_object_get_string(object, "cullface");

        block_face parsed;

        if(!parse_block_face_name(name, parsed)) {
            return false;
        }

        face.cull_face = parsed;
    }

    if(json_object_has_value(object, "tint")) {
        auto raw = json_object_get_number(object, "tint");

        if(raw < 0.0) {
            return false;
        }

        face.tint_index = static_cast<unsigned>(raw);
    }

    face.world_locked = false;

    if(json_object_has_value(object, "world_locked")) {
        face.world_locked = json_object_get_boolean(object, "world_locked");
    }

    return true;
}

static bool parse_element(BlockModel_Element& element, const JSON_Object* object)
{
    if(object == nullptr) {
        return false;
    }

    auto is_valid = true;
    is_valid = is_valid && utils::parse_json(object, "from", element.from);
    is_valid = is_valid && utils::parse_json(object, "to", element.to);

    if(!is_valid) {
        return false;
    }

    element.from /= 16.0f;
    element.to /= 16.0f;

    element.rotation_origin = Eigen::Vector3f::Zero();

    if(json_object_has_value(object, "origin")) {
        if(!utils::parse_json(object, "origin", element.rotation_origin)) {
            return false;
        }

        element.rotation_origin /= 16.0f;
    }

    element.rotation_angles = Eigen::Vector3f::Zero();

    if(json_object_has_value(object, "rotation")) {
        if(!utils::parse_json(object, "rotation", element.rotation_angles)) {
            return false;
        }
    }

    element.rotation_angles[0] = snap_rotation(element.rotation_angles[0]);
    element.rotation_angles[1] = snap_rotation(element.rotation_angles[1]);
    element.rotation_angles[2] = snap_rotation(element.rotation_angles[2]);

    element.rescale = true;

    if(json_object_has_value(object, "rescale")) {
        element.rescale = json_object_get_boolean(object, "rescale");
    }

    element.shade = true;

    if(json_object_has_value(object, "shade")) {
        element.shade = json_object_get_boolean(object, "shade");
    }

    element.faces.fill(std::nullopt);

    auto faces = json_object_get_object(object, "faces");

    if(faces == nullptr) {
        return false;
    }

    auto num_faces = json_object_get_count(faces);

    for(std::size_t i = 0; i < num_faces; ++i) {
        auto face_name = json_object_get_name(faces, i);
        auto face_object = json_object_get_object(faces, face_name);

        if(face_object == nullptr) {
            return false;
        }

        block_face face_index;

        if(!parse_block_face_name(face_name, face_index)) {
            return false;
        }

        BlockModel_Face face {};

        if(!parse_face(face, face_object)) {
            return false;
        }

        element.faces[face_index] = std::move(face);
    }

    return true;
}

static const void* block_model_load_fn(const char* path, std::uint32_t flags)
{
    std::string source;

    if(!utils::read_file(path, source)) {
        LOG_WARNING("{}: {}", path, utils::physfs_error());
        return nullptr;
    }

    auto json = json_parse_string(source.c_str());
    auto jsonv = json_value_get_object(json);

    if(json == nullptr || jsonv == nullptr) {
        LOG_WARNING("{}: parse error", path);
        json_value_free(json);
        return nullptr;
    }

    BlockModel model {};

    auto textures = json_object_get_array(jsonv, "textures");

    if(!parse_texture_slots(model, textures)) {
        LOG_WARNING("{}: invalid or missing 'textures'", path);
        json_value_free(json);
        return nullptr;
    }

    auto elements = json_object_get_array(jsonv, "elements");
    auto num_elements = elements ? json_array_get_count(elements) : 0;

    if(elements == nullptr || num_elements == 0) {
        LOG_WARNING("{}: invalid or missing 'elements'", path);
        json_value_free(json);
        return nullptr;
    }

    model.elements.reserve(num_elements);

    for(std::size_t i = 0; i < num_elements; ++i) {
        auto element_object = json_array_get_object(elements, i);

        BlockModel_Element element {};

        if(!parse_element(element, element_object)) {
            LOG_WARNING("{}: invalid element at index {}", path, i);
            json_value_free(json);
            return nullptr;
        }

        model.elements.push_back(std::move(element));
    }

    json_value_free(json);

    return new BlockModel(std::move(model));
}

static void block_model_free_fn(const void* data)
{
    delete reinterpret_cast<const BlockModel*>(data);
}

void BlockModel::register_resource(void)
{
    res::register_loader<BlockModel>(&block_model_load_fn, &block_model_free_fn);
}
