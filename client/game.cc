#include "client/pch.hh"

#include "client/game.hh"

#include "core/res/resource.hh"

#include "core/buffer.hh"
#include "core/exception.hh"
#include "core/identifier.hh"

#include "shared/res/block_model.hh"

#include "shared/utils/coord.hh"
#include "shared/utils/lua.hh"

#include "shared/block_registry.hh"
#include "shared/block_storage.hh"
#include "shared/coord.hh"
#include "shared/mod_loader.hh"
#include "shared/world.hh"

#include "client/block_models.hh"

#include "client/res/texture2D.hh"

#include "client/globals.hh"

static res::handle<Texture2D> s_texture;

static void test_block_storage(void)
{
    auto definitions = block_registry::all_definitions();

    if(definitions.size() < 3) {
        LOG_WARNING("test_block_storage: not enough block definitions loaded, skipping");
        return;
    }

    const block_id_type id_a = 1;
    const block_id_type id_b = 2;

    BlockStorage storage;

    // fresh storage should be a Uniform of BLOCK_ID_NULL everywhere
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; i += 4096) {
        vx::throw_if_not_fmt(storage.get(i) == BLOCK_ID_NULL, "test_block_storage: fresh storage not null at {}", i);
    }

    // Uniform -> Palette8 promotion
    storage.set(local_pos(0, 0, 0), id_a);
    vx::throw_if_not_fmt(storage.get(local_pos(0, 0, 0)) == id_a, "test_block_storage: id_a readback failed after promotion");
    vx::throw_if_not_fmt(storage.get(local_pos(1, 0, 0)) == BLOCK_ID_NULL, "test_block_storage: neighbor corrupted after promotion");

    // fill enough of the chunk with a second id to exercise refcounting
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME / 2; ++i) {
        storage.set(i, id_b);
    }

    vx::throw_if_not_fmt(storage.get(std::size_t { 0 }) == id_b, "test_block_storage: id_b readback failed");
    vx::throw_if_not_fmt(storage.get(constant::CHUNK_VOLUME - 1) == BLOCK_ID_NULL,
        "test_block_storage: untouched tail should still be null");

    // force Palette8 -> Palette16 promotion by exhausting the 8-bit palette
    for(block_id_type id = 3; id < 3 + 300 && id < definitions.size(); ++id) {
        storage.set(static_cast<std::size_t>(id), id);
    }

    for(block_id_type id = 3; id < 3 + 300 && id < definitions.size(); ++id) {
        vx::throw_if_not_fmt(storage.get(static_cast<std::size_t>(id)) == id, "test_block_storage: palette16 readback failed for id {}",
            id);
    }

    vx::throw_if_not_fmt(storage.size() > 0, "test_block_storage: size() reported zero for a non-uniform chunk");

    // round-trip through the serializer
    WriteBuffer wbuf;
    BlockStorage::serialize(storage, wbuf);

    ReadBuffer rbuf(wbuf.data(), wbuf.size());
    BlockStorage restored;
    BlockStorage::deserialize(restored, rbuf);

    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; i += 997) {
        vx::throw_if_not_fmt(storage.get(i) == restored.get(i), "test_block_storage: serialize round-trip mismatch at {}", i);
    }

    // optimize() should collapse back down once the higher ids are overwritten
    for(block_id_type id = 3; id < 3 + 300 && id < definitions.size(); ++id) {
        storage.set(static_cast<std::size_t>(id), id_b);
    }

    storage.optimize();

    for(std::size_t i = 0; i < constant::CHUNK_VOLUME / 2; i += 4096) {
        vx::throw_if_not_fmt(storage.get(i) == id_b, "test_block_storage: optimize() readback failed at {}", i);
    }

    vx::throw_if_not_fmt(storage.get(constant::CHUNK_VOLUME - 1) == BLOCK_ID_NULL,
        "test_block_storage: optimize() corrupted untouched tail");

    LOG_INFO("test_block_storage: OK");
}

static void test_world(void)
{
    auto slab_id = block_registry::find(Identifier::from_string("builtin:stone_slab"));

    if(slab_id == BLOCK_ID_NULL) {
        LOG_WARNING("test_world: builtin:stone_slab not found, skipping");
        return;
    }

    const chunk_pos cpos(0, 0, 0);
    const local_pos lpos(1, 2, 3);
    const block_pos bpos = utils::to_block(cpos, lpos);

    // no chunk yet - everything should read back as empty/absent
    vx::throw_if_not_fmt(world::get_block(bpos) == BLOCK_ID_NULL, "test_world: get_block on missing chunk should be null");
    vx::throw_if_not_fmt(!world::set_block(bpos, slab_id), "test_world: set_block on missing chunk should fail");
    vx::throw_if_not_fmt(!world::get_state(bpos, "orientation").has_value(), "test_world: get_state on missing chunk should be nullopt");

    world::create_chunk(cpos);

    vx::throw_if_not_fmt(world::set_block(bpos, slab_id), "test_world: set_block should succeed once the chunk exists");
    vx::throw_if_not_fmt(world::get_block(bpos) == slab_id, "test_world: get_block readback mismatch");

    // "bottom" is the declared default - id was never explicitly resolved with this
    // state set, so get_state should fall back to BlockStateDecl::default_value
    auto orientation = world::get_state(bpos, "orientation");
    vx::throw_if_not_fmt(orientation.has_value() && orientation.value() == "bottom", "test_world: expected default orientation 'bottom'");

    vx::throw_if_not_fmt(world::set_state(bpos, "orientation", "top"), "test_world: set_state failed");

    auto new_id = world::get_block(bpos);
    vx::throw_if_not_fmt(new_id != slab_id, "test_world: set_state should have resolved a distinct variant id");

    orientation = world::get_state(bpos, "orientation");
    vx::throw_if_not_fmt(orientation.has_value() && orientation.value() == "top",
        "test_world: orientation readback after set_state failed");

    vx::throw_if_not_fmt(!world::get_state(bpos, "no_such_state").has_value(), "test_world: unknown state name should return nullopt");

    // scheduling
    world::schedule(bpos, 4);
    world::current_tick = 0;

    for(std::size_t i = 0; i < 4; ++i) {
        world::fixed_update();
    }

    // fixed_update just drains pop_due internally for now (no BlockCallback dispatch
    // yet), so this only proves the tick counter and pop_due plumbing don't crash
    vx::throw_if_not_fmt(world::current_tick == 4, "test_world: current_tick mismatch after fixed_update loop");

    world::remove_chunk(cpos);
    vx::throw_if_not_fmt(world::find_chunk(cpos) == nullptr, "test_world: chunk should be gone after remove_chunk");

    LOG_INFO("test_world: OK");
}

static const char* block_face_name(block_face face) noexcept
{
    switch(face) {
        case BLOCK_FACE_NORTH:
            return "north";
        case BLOCK_FACE_SOUTH:
            return "south";
        case BLOCK_FACE_EAST:
            return "east";
        case BLOCK_FACE_WEST:
            return "west";
        case BLOCK_FACE_TOP:
            return "top";
        case BLOCK_FACE_BOTTOM:
            return "bottom";
        default:
            return "unknown";
    }
}

static void dump_block_model(const Identifier& id, const BlockModel& model)
{
    LOG_DEBUG("block model dump: {}", id.full_string());
    LOG_DEBUG("  textures: {}", model.texture_slots.size());

    for(const auto& slot : model.texture_slots) {
        LOG_DEBUG("    - {}", slot);
    }

    LOG_DEBUG("  elements: {}", model.elements.size());

    for(std::size_t i = 0; i < model.elements.size(); ++i) {
        const auto& element = model.elements[i];

        LOG_DEBUG("    [{}] from=({}, {}, {}) to=({}, {}, {})", i, element.from.x(), element.from.y(), element.from.z(), element.to.x(),
            element.to.y(), element.to.z());
        LOG_DEBUG("    [{}] origin=({}, {}, {}) rotation=({}, {}, {}) rescale={} shade={}", i, element.rotation_origin.x(),
            element.rotation_origin.y(), element.rotation_origin.z(), element.rotation_angles.x(), element.rotation_angles.y(),
            element.rotation_angles.z(), element.rescale, element.shade);

        for(std::size_t j = 0; j < element.faces.size(); ++j) {
            const auto& face = element.faces[j];

            if(!face.has_value()) {
                continue;
            }

            LOG_DEBUG("    [{}] face {}: texture={} uv_rotation={} cullface={} tint={}", i, block_face_name(static_cast<block_face>(j)),
                face->texture_slot, face->uv_rotation, face->cull_face.has_value() ? block_face_name(face->cull_face.value()) : "none",
                face->tint_index.has_value() ? std::to_string(face->tint_index.value()) : "none");
        }
    }
}

static void test_block_model(void)
{
    auto id = Identifier::from_string("builtin:cube");
    auto model = res::load<BlockModel>(id, "models/block", ".json");

    if(model == nullptr) {
        LOG_WARNING("test_block_model: builtin:cube failed to load, skipping");
        return;
    }

    dump_block_model(id, *model);

    vx::throw_if_not_fmt(model->texture_slots.size() == 6, "test_block_model: expected 6 texture slots, got {}",
        model->texture_slots.size());
    vx::throw_if_not_fmt(model->elements.size() == 1, "test_block_model: expected 1 element, got {}", model->elements.size());

    const auto& element = model->elements.front();

    vx::throw_if_not_fmt(element.from == Eigen::Vector3f::Zero(), "test_block_model: expected from = [0, 0, 0]");
    vx::throw_if_not_fmt(element.to == Eigen::Vector3f::Ones(), "test_block_model: expected to = [1, 1, 1]");
    vx::throw_if_not_fmt(element.rescale, "test_block_model: rescale should default to true");
    vx::throw_if_not_fmt(element.shade, "test_block_model: shade should default to true");

    for(auto i = 0; i < 6; ++i) {
        vx::throw_if_not_fmt(element.faces[i].has_value(), "test_block_model: face {} should be present", i);
    }

    vx::throw_if_not_fmt(element.faces[BLOCK_FACE_TOP]->texture_slot == "top", "test_block_model: top face texture slot mismatch");
    vx::throw_if_not_fmt(element.faces[BLOCK_FACE_TOP]->cull_face == BLOCK_FACE_BOTTOM,
        "test_block_model: top face should cullface bottom");

    // this model shouldn't stick around in the resource cache: it wasn't loaded with RESFLAG_CACHE
    model.reset();
    res::soft_purge(true);
    vx::throw_if_not_fmt(res::find<BlockModel>(id, "models/block", ".json") == nullptr,
        "test_block_model: model should be gone after soft_purge");

    LOG_INFO("test_block_model: OK");
}

static void dump_baked_block_model(const std::string& label, block_id_type id)
{
    auto baked = block_models::find(id);

    if(baked == nullptr) {
        LOG_WARNING("test_block_models: {}: no baked model", label);
        return;
    }

    std::size_t total = baked->unculled_quads.size();

    LOG_DEBUG("test_block_models: {}", label);
    LOG_DEBUG("  unculled_quads: {}", baked->unculled_quads.size());

    const BakedBlockModel_Quad* sample = baked->unculled_quads.empty() ? nullptr : &baked->unculled_quads.front();

    for(auto face : { BLOCK_FACE_NORTH, BLOCK_FACE_SOUTH, BLOCK_FACE_EAST, BLOCK_FACE_WEST, BLOCK_FACE_TOP, BLOCK_FACE_BOTTOM }) {
        total += baked->face_quads[face].size();

        LOG_DEBUG("  face_quads[{}]: {} fully_covered={}", block_face_name(face), baked->face_quads[face].size(),
            baked->fully_covered[face]);

        if(sample == nullptr && !baked->face_quads[face].empty()) {
            sample = &baked->face_quads[face].front();
        }
    }

    vx::throw_if_not_fmt(total != 0, "test_block_models: {}: baked model has zero quads", label);

    LOG_DEBUG("  sample quad: pos0=({}, {}, {}) uv0=({}, {}) packed_normal={:#010x} frame_base={} frame_count={} tint={} shade={}",
        sample->positions[0].x(), sample->positions[0].y(), sample->positions[0].z(), sample->uvs[0].x(), sample->uvs[0].y(),
        sample->packed_normal, sample->frame_base, sample->frame_count, sample->tint_index, sample->shade);
}

static void test_block_models(void)
{
    constexpr std::array names = { "builtin:dirt", "builtin:grass", "builtin:stone", "builtin:stone_slab" };

    for(const char* name : names) {
        auto base_id = block_registry::find(Identifier::from_string(name));

        if(base_id == BLOCK_ID_NULL) {
            LOG_WARNING("test_block_models: {}: not registered, skipping", name);
            continue;
        }

        auto family = block_registry::find_family_of(base_id);

        if(family == nullptr) {
            dump_baked_block_model(std::format("{} (id={})", name, base_id), base_id);
            continue;
        }

        std::vector<block_id_type> ids;

        for(const auto& [id, states] : family->id_states) {
            ids.push_back(id);
        }

        std::sort(ids.begin(), ids.end());

        for(auto id : ids) {
            std::string state_str;

            for(const auto& [key, value] : family->id_states.at(id)) {
                if(!state_str.empty()) {
                    state_str += ",";
                }

                state_str += family->state_value(value);
            }

            dump_baked_block_model(std::format("{} (id={}) states=[{}]", name, id, state_str), id);
        }
    }

    LOG_INFO("test_block_models: OK");
}

static void test_scripting_world(void)
{
    auto mod = mod_loader::find(BUILTIN_MOD_NAME);

    if(!mod || !mod->lua_state()) {
        LOG_WARNING("test_scripting_world: builtin mod not loaded, skipping");
        return;
    }

    const chunk_pos cpos(0, 0, 0);

    world::create_chunk(cpos);

    static constexpr const char* SCRIPT = R"lua(
        print("test_scripting_world: begin")

        local bx, by, bz = 1, 2, 3
        local slab = assert(blocks.get("builtin:stone_slab"), "builtin:stone_slab missing")
        print("test_scripting_world: builtin:stone_slab id =", slab)

        assert(world.get_block(bx, by, bz) == blocks.NULL_BLOCK, "fresh chunk should read back as void")
        print("test_scripting_world: fresh block is void, OK")

        world.set_block(bx, by, bz, slab)
        assert(world.get_block(bx, by, bz) == slab, "set_block/get_block readback mismatch")
        print("test_scripting_world: set_block/get_block roundtrip, OK")

        assert(world.get_state(bx, by, bz, "orientation") == "bottom", "expected default orientation 'bottom'")
        print("test_scripting_world: default orientation is 'bottom', OK")

        world.set_state(bx, by, bz, "orientation", "top")
        assert(world.get_state(bx, by, bz, "orientation") == "top", "set_state/get_state readback mismatch")
        print("test_scripting_world: set_state/get_state roundtrip, OK")

        assert(world.get_state(bx, by, bz, "no_such_state") == nil, "unknown state should read back as nil")
        print("test_scripting_world: unknown state reads back as nil, OK")

        world.set_block(bx, by, bz, blocks.NULL_BLOCK)
        assert(world.get_block(bx, by, bz) == blocks.NULL_BLOCK, "set_block(NULL_BLOCK) should clear the block")
        print("test_scripting_world: set_block(NULL_BLOCK) clears the block, OK")

        print("test_scripting_world: end")
    )lua";

    auto L = mod->lua_state().get();

    auto load_status = luaL_loadstring(L, SCRIPT);

    if(load_status != LUA_OK) {
        auto message = lua_tostring(L, -1);
        world::remove_chunk(cpos);
        throw vx::runtime_error("test_scripting_world: {}", message);
    }

    auto pcall_status = lua_pcall(L, 0, 0, 0);

    if(pcall_status != LUA_OK) {
        auto message = lua_tostring(L, -1);
        world::remove_chunk(cpos);
        throw vx::runtime_error("test_scripting_world: {}", message);
    }

    world::remove_chunk(cpos);

    LOG_INFO("test_scripting_world: OK");
}

void client_game::init(void)
{
    Identifier i;

    i = Identifier::from_string("voxelius:some_identifier");
    LOG_DEBUG("{}", i.full_string());

    i = Identifier::from_string(":invalid_identifier");
    LOG_DEBUG("{} {}", i.full_string(), i.is_valid());

    i = Identifier::from_string("something", "default_namespace");
    LOG_DEBUG("{} {}", i.full_string(), i.is_valid());

    i = Identifier::from_string("voxelius:something", "default_namespace");
    LOG_DEBUG("{}", i.full_string());

    i = Identifier::from_string("builtin:cube");
    LOG_DEBUG("{}", i.as_file_path("models/block", ".json"));

    s_texture = res::load<Texture2D>(Identifier::from_string("builtin:textures/trollface.png"));
}

void client_game::init_late(void)
{
    test_block_storage();
    test_block_model();
    test_block_models();
    test_world();
    test_scripting_world();
}

void client_game::shutdown(void)
{
    s_texture.reset();
}

void client_game::update(void)
{
    // empty
}

void client_game::update_late(void)
{
    // empty
}

void client_game::fixed_update(void)
{
    // empty
}

void client_game::fixed_update_late(void)
{
    // empty
}

void client_game::layout(void)
{
    ImGui::Text("skibidi sigma %f", 1.0f / globals::window_frametime_avg);

    ImGui::Image(s_texture->imgui, ImVec2(320.0f, 240.0f));
}
