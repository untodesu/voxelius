#include "client/pch.hh"

#include "client/game.hh"

#include "core/res/resource.hh"

#include "core/buffer.hh"
#include "core/exception.hh"
#include "core/identifier.hh"

#include "shared/block_registry.hh"
#include "shared/block_storage.hh"
#include "shared/coord.hh"

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
    vx::throw_if_not_fmt(
        storage.get(constant::CHUNK_VOLUME - 1) == BLOCK_ID_NULL, "test_block_storage: untouched tail should still be null");

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

    vx::throw_if_not_fmt(
        storage.get(constant::CHUNK_VOLUME - 1) == BLOCK_ID_NULL, "test_block_storage: optimize() corrupted untouched tail");

    LOG_INFO("test_block_storage: OK");
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
