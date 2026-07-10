#include "client/pch.hh"

#include "client/block_models.hh"

#include "core/res/resource.hh"

#include "core/utils/angles.hh"

#include "shared/res/block_model.hh"

#include "shared/block_registry.hh"

#include "client/block_atlas.hh"

constexpr static std::array ALL_FACES = {
    BLOCK_FACE_NORTH,
    BLOCK_FACE_SOUTH,
    BLOCK_FACE_EAST,
    BLOCK_FACE_WEST,
    BLOCK_FACE_TOP,
    BLOCK_FACE_BOTTOM,
};

constexpr static std::array FACE_ROTATIONS = {
    std::array { BLOCK_FACE_NORTH, BLOCK_FACE_SOUTH, BLOCK_FACE_EAST, BLOCK_FACE_WEST, BLOCK_FACE_TOP, BLOCK_FACE_BOTTOM },
    std::array { BLOCK_FACE_SOUTH, BLOCK_FACE_NORTH, BLOCK_FACE_WEST, BLOCK_FACE_EAST, BLOCK_FACE_TOP, BLOCK_FACE_BOTTOM },
    std::array { BLOCK_FACE_EAST, BLOCK_FACE_WEST, BLOCK_FACE_SOUTH, BLOCK_FACE_NORTH, BLOCK_FACE_TOP, BLOCK_FACE_BOTTOM },
    std::array { BLOCK_FACE_WEST, BLOCK_FACE_EAST, BLOCK_FACE_NORTH, BLOCK_FACE_SOUTH, BLOCK_FACE_TOP, BLOCK_FACE_BOTTOM },
    std::array { BLOCK_FACE_TOP, BLOCK_FACE_BOTTOM, BLOCK_FACE_EAST, BLOCK_FACE_WEST, BLOCK_FACE_SOUTH, BLOCK_FACE_NORTH },
    std::array { BLOCK_FACE_BOTTOM, BLOCK_FACE_TOP, BLOCK_FACE_EAST, BLOCK_FACE_WEST, BLOCK_FACE_NORTH, BLOCK_FACE_SOUTH },
};

static std::vector<std::unique_ptr<BakedBlockModel>> s_models;

static Eigen::Vector3f face_normal(block_face face) noexcept
{
    switch(face) {
        case BLOCK_FACE_NORTH:
            return -Eigen::Vector3f::UnitZ();

        case BLOCK_FACE_SOUTH:
            return Eigen::Vector3f::UnitZ();

        case BLOCK_FACE_EAST:
            return Eigen::Vector3f::UnitX();

        case BLOCK_FACE_WEST:
            return -Eigen::Vector3f::UnitX();

        case BLOCK_FACE_TOP:
            return Eigen::Vector3f::UnitY();

        case BLOCK_FACE_BOTTOM:
            return -Eigen::Vector3f::UnitY();
    }

    return Eigen::Vector3f::Zero();
}

static block_face rotate_face(block_face face, block_face facing) noexcept
{
    return FACE_ROTATIONS[facing][face];
}

static Eigen::Matrix3f facing_rotation(block_face facing) noexcept
{
    constexpr auto pi = std::numbers::pi_v<float>;

    switch(facing) {
        case BLOCK_FACE_SOUTH:
            return Eigen::AngleAxisf(pi, Eigen::Vector3f::UnitY()).toRotationMatrix();

        case BLOCK_FACE_EAST:
            return Eigen::AngleAxisf(-pi * 0.5f, Eigen::Vector3f::UnitY()).toRotationMatrix();

        case BLOCK_FACE_WEST:
            return Eigen::AngleAxisf(pi * 0.5f, Eigen::Vector3f::UnitY()).toRotationMatrix();

        case BLOCK_FACE_TOP:
            return Eigen::AngleAxisf(-pi * 0.5f, Eigen::Vector3f::UnitX()).toRotationMatrix();

        case BLOCK_FACE_BOTTOM:
            return Eigen::AngleAxisf(pi * 0.5f, Eigen::Vector3f::UnitX()).toRotationMatrix();
    }

    return Eigen::Matrix3f::Identity();
}

static Eigen::Matrix3f element_rotation(const Eigen::Vector3f& angles) noexcept
{
    auto radians = utils::radians(angles);

    auto rx = Eigen::AngleAxisf(radians.x(), Eigen::Vector3f::UnitX());
    auto ry = Eigen::AngleAxisf(radians.y(), Eigen::Vector3f::UnitY());
    auto rz = Eigen::AngleAxisf(radians.z(), Eigen::Vector3f::UnitZ());

    return (rz * ry * rx).toRotationMatrix();
}

static std::uint32_t pack_normal_2_10_10_10(const Eigen::Vector3f& normal) noexcept
{
    auto snx = static_cast<std::int32_t>(std::clamp(normal.x() * 511.0f, -511.0f, 511.0f));
    auto sny = static_cast<std::int32_t>(std::clamp(normal.y() * 511.0f, -511.0f, 511.0f));
    auto snz = static_cast<std::int32_t>(std::clamp(normal.z() * 511.0f, -511.0f, 511.0f));

    auto nx = static_cast<std::uint32_t>(snx) & 0x3FF;
    auto ny = static_cast<std::uint32_t>(sny) & 0x3FF;
    auto nz = static_cast<std::uint32_t>(snz) & 0x3FF;

    return (nz << 20) | (ny << 10) | nx;
}

static void make_face_geometry(const Eigen::Vector3f& from, const Eigen::Vector3f& to, block_face face, BakedBlockModel_Quad& quad) noexcept
{
    quad.positions.fill(Eigen::Vector3f::Zero());

    switch(face) {
        case BLOCK_FACE_NORTH:
            quad.positions[0] = Eigen::Vector3f(from.x(), to.y(), from.z());
            quad.positions[1] = Eigen::Vector3f(to.x(), to.y(), from.z());
            quad.positions[2] = Eigen::Vector3f(to.x(), from.y(), from.z());
            quad.positions[3] = Eigen::Vector3f(from.x(), from.y(), from.z());
            break;

        case BLOCK_FACE_SOUTH:
            quad.positions[0] = Eigen::Vector3f(to.x(), to.y(), to.z());
            quad.positions[1] = Eigen::Vector3f(from.x(), to.y(), to.z());
            quad.positions[2] = Eigen::Vector3f(from.x(), from.y(), to.z());
            quad.positions[3] = Eigen::Vector3f(to.x(), from.y(), to.z());
            break;

        case BLOCK_FACE_EAST:
            quad.positions[0] = Eigen::Vector3f(to.x(), to.y(), from.z());
            quad.positions[1] = Eigen::Vector3f(to.x(), to.y(), to.z());
            quad.positions[2] = Eigen::Vector3f(to.x(), from.y(), to.z());
            quad.positions[3] = Eigen::Vector3f(to.x(), from.y(), from.z());
            break;

        case BLOCK_FACE_WEST:
            quad.positions[0] = Eigen::Vector3f(from.x(), to.y(), to.z());
            quad.positions[1] = Eigen::Vector3f(from.x(), to.y(), from.z());
            quad.positions[2] = Eigen::Vector3f(from.x(), from.y(), from.z());
            quad.positions[3] = Eigen::Vector3f(from.x(), from.y(), to.z());
            break;

        case BLOCK_FACE_TOP:
            quad.positions[0] = Eigen::Vector3f(from.x(), to.y(), from.z());
            quad.positions[1] = Eigen::Vector3f(from.x(), to.y(), to.z());
            quad.positions[2] = Eigen::Vector3f(to.x(), to.y(), to.z());
            quad.positions[3] = Eigen::Vector3f(to.x(), to.y(), from.z());
            break;

        case BLOCK_FACE_BOTTOM:
            quad.positions[0] = Eigen::Vector3f(from.x(), from.y(), to.z());
            quad.positions[1] = Eigen::Vector3f(from.x(), from.y(), from.z());
            quad.positions[2] = Eigen::Vector3f(to.x(), from.y(), from.z());
            quad.positions[3] = Eigen::Vector3f(to.x(), from.y(), to.z());
            break;
    }

    quad.uvs[0] = Eigen::Vector2f(0.0f, 0.0f);
    quad.uvs[1] = Eigen::Vector2f(1.0f, 0.0f);
    quad.uvs[2] = Eigen::Vector2f(1.0f, 1.0f);
    quad.uvs[3] = Eigen::Vector2f(0.0f, 1.0f);
}

static void apply_face_uv(const BlockModel_Face& face, BakedBlockModel_Quad& quad) noexcept
{
    if(face.uv.has_value()) {
        quad.uvs[0] = Eigen::Vector2f(face.uv->x(), face.uv->y());
        quad.uvs[1] = Eigen::Vector2f(face.uv->z(), face.uv->y());
        quad.uvs[2] = Eigen::Vector2f(face.uv->z(), face.uv->w());
        quad.uvs[3] = Eigen::Vector2f(face.uv->x(), face.uv->w());
    }

    auto steps = static_cast<std::ptrdiff_t>(face.uv_rotation / 90);

    std::rotate(quad.uvs.begin(), quad.uvs.begin() + steps, quad.uvs.end());
}

static bool is_covering(const BlockModel_Element& element, block_face face) noexcept
{
    auto result = true;

    switch(face) {
        case BLOCK_FACE_NORTH:
            result = result && element.from.x() <= 0.0f && element.to.x() >= 1.0f;
            result = result && element.from.y() <= 0.0f && element.to.y() >= 1.0f;
            break;

        case BLOCK_FACE_SOUTH:
            result = result && element.from.x() <= 0.0f && element.to.x() >= 1.0f;
            result = result && element.from.y() <= 0.0f && element.to.y() >= 1.0f;
            break;

        case BLOCK_FACE_EAST:
            result = result && element.from.z() <= 0.0f && element.to.z() >= 1.0f;
            result = result && element.from.y() <= 0.0f && element.to.y() >= 1.0f;
            break;

        case BLOCK_FACE_WEST:
            result = result && element.from.z() <= 0.0f && element.to.z() >= 1.0f;
            result = result && element.from.y() <= 0.0f && element.to.y() >= 1.0f;
            break;

        case BLOCK_FACE_TOP:
            result = result && element.from.x() <= 0.0f && element.to.x() >= 1.0f;
            result = result && element.from.z() <= 0.0f && element.to.z() >= 1.0f;
            break;

        case BLOCK_FACE_BOTTOM:
            result = result && element.from.x() <= 0.0f && element.to.x() >= 1.0f;
            result = result && element.from.z() <= 0.0f && element.to.z() >= 1.0f;
            break;

        default:
            result = false;
            break;
    }

    return result && element.rotation_angles.isZero();
}

static std::unique_ptr<BakedBlockModel> bake_model(const BlockDefinition& def) noexcept
{
    if(def.is_stem || def.model_name.is_empty()) {
        return nullptr;
    }

    auto model = res::load<BlockModel>(def.model_name, "models/block", ".json");

    if(model == nullptr) {
        LOG_WARNING("{}: load failed", def.model_name.full_string());
        return nullptr;
    }

    auto baked = std::make_unique<BakedBlockModel>();
    baked->fully_covered.fill(false);

    auto facing_rot = facing_rotation(def.model_facing);
    auto center = Eigen::Vector3f::Constant(0.5f);

    for(auto& element : model->elements) {
        auto element_rot = element_rotation(element.rotation_angles);

        for(auto face : ALL_FACES) {
            if(!element.faces[face].has_value()) {
                continue;
            }

            auto& face_def = element.faces[face].value();
            auto rotated_face = rotate_face(face, def.model_facing);

            auto texture_face = &face_def;

            if(face_def.world_locked && element.faces[rotated_face].has_value()) {
                texture_face = &element.faces[rotated_face].value();
            }

            auto frames = def.resolve_texture_slot(texture_face->texture_slot);

            if(!frames.has_value()) {
                LOG_WARNING("{}: {}: missing textures", def.model_name.full_string(), texture_face->texture_slot);
                continue;
            }

            auto strip = block_atlas::find(frames.value());

            if(strip == nullptr) {
                LOG_WARNING("{}: {}: atlas strip not found", def.model_name.full_string(), texture_face->texture_slot);
                continue;
            }

            BakedBlockModel_Quad quad {};
            make_face_geometry(element.from, element.to, face, quad);
            apply_face_uv(face_def, quad);

            quad.frame_base = strip->frame_base;
            quad.frame_count = strip->frame_count;
            quad.tint_index = face_def.tint_index.value_or(0);
            quad.shade = element.shade;

            for(auto& position : quad.positions) {
                position = element_rot * (position - element.rotation_origin) + element.rotation_origin;
                position = facing_rot * (position - center) + center;
                position += def.model_offset;
            }

            quad.packed_normal = pack_normal_2_10_10_10(facing_rot * (element_rot * face_normal(face)));

            if(face_def.cull_face.has_value()) {
                baked->face_quads[rotated_face].push_back(quad);

                if(is_covering(element, face)) {
                    baked->fully_covered[rotated_face] = true;
                }
            }
            else {
                baked->unculled_quads.push_back(quad);
            }
        }
    }

    return baked;
}

void block_models::init_late(void)
{
    auto definitions = block_registry::all_definitions();

    s_models.clear();
    s_models.resize(definitions.size());

    for(block_id_type id = 0; id < definitions.size(); id += 1) {
        s_models[id] = bake_model(definitions[id]);
    }
}

void block_models::shutdown(void)
{
    s_models.clear();
}

const BakedBlockModel* block_models::find(block_id_type id) noexcept
{
    if(id == BLOCK_ID_NULL || id >= s_models.size()) {
        return nullptr;
    }

    return s_models[id].get();
}
