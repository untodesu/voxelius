#include "shared/pch.hh"

#include "shared/world.hh"

#include "shared/block_registry.hh"
#include "shared/utils/coord.hh"

#include "shared/globals.hh"

emhash8::HashMap<chunk_pos, std::shared_ptr<Chunk>> world::chunks;
entt::registry world::basic_entities;
entt::registry world::chunk_entities;
std::uint64_t world::current_tick = 0;

ChunkCreateEvent::ChunkCreateEvent(const chunk_pos& pos, const std::shared_ptr<Chunk>& chunk) : m_chunk(chunk), m_pos(pos)
{
    // empty
}

ChunkRemoveEvent::ChunkRemoveEvent(const chunk_pos& pos, const std::shared_ptr<const Chunk>& chunk) : m_chunk(chunk), m_pos(pos)
{
    // empty
}

ChunkUpdateEvent::ChunkUpdateEvent(const chunk_pos& pos, const std::shared_ptr<Chunk>& chunk) : m_chunk(chunk), m_pos(pos)
{
    // empty
}

BlockUpdateEvent::BlockUpdateEvent(const block_pos& pos, block_id_type id, const std::shared_ptr<Chunk>& chunk)
    : m_chunk(chunk), m_id(id), m_bpos(pos), m_cpos(utils::to_chunk(pos)), m_lpos(utils::to_local(pos))
{
    // empty
}

std::shared_ptr<Chunk> world::create_chunk(const chunk_pos& pos)
{
    auto it = chunks.find(pos);

    if(it == chunks.cend()) {
        auto entity = chunk_entities.create();
        auto chunk = std::make_shared<Chunk>(entity);

        auto& component = chunk_entities.emplace<ChunkComponent>(entity);
        component.position = pos;
        component.ptr = chunk;

        chunks.insert_or_assign(chunk_pos(pos), std::shared_ptr(chunk));

        globals::dispatcher.trigger(ChunkCreateEvent(pos, chunk));

        return chunk;
    }

    return it->second;
}

std::shared_ptr<Chunk> world::find_chunk(const chunk_pos& pos)
{
    auto it = chunks.find(pos);

    if(it == chunks.cend()) {
        return nullptr;
    }

    return it->second;
}

std::shared_ptr<Chunk> world::find_chunk(entt::entity entity)
{
    if(chunk_entities.valid(entity)) {
        auto& component = chunk_entities.get<ChunkComponent>(entity);
        return component.ptr;
    }

    return nullptr;
}

void world::remove_chunk(const std::shared_ptr<const Chunk>& chunk)
{
    if(chunk) {
        remove_chunk(chunk->entity());
    }
}

void world::remove_chunk(const chunk_pos& pos)
{
    auto it = chunks.find(pos);

    if(it == chunks.cend()) {
        return;
    }

    globals::dispatcher.trigger(ChunkRemoveEvent(pos, it->second));

    chunk_entities.destroy(it->second->entity());
    chunks.erase(it);
}

void world::remove_chunk(entt::entity entity)
{
    if(chunk_entities.valid(entity)) {
        const auto& component = chunk_entities.get<ChunkComponent>(entity);

        globals::dispatcher.trigger(ChunkRemoveEvent(component.position, component.ptr));

        chunks.erase(component.position);
        chunk_entities.destroy(entity);
    }
}

block_id_type world::get_block(const chunk_pos& cpos, const local_pos& lpos)
{
    if(auto chunk = find_chunk(cpos)) {
        return chunk->get_block(lpos);
    }

    return BLOCK_ID_NULL;
}

block_id_type world::get_block(const block_pos& pos)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return get_block(cpos, lpos);
}

bool world::set_block(const chunk_pos& cpos, const local_pos& lpos, block_id_type id)
{
    if(auto chunk = find_chunk(cpos)) {
        chunk->set_block(lpos, id);

        globals::dispatcher.trigger(BlockUpdateEvent(utils::to_block(cpos, lpos), id, chunk));

        return true;
    }

    return false;
}

bool world::set_block(const block_pos& pos, block_id_type id)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return set_block(cpos, lpos, id);
}

block_light_type world::get_light(const chunk_pos& cpos, const local_pos& lpos)
{
    return BLOCK_LIGHT_MIN; // TODO: lightmaps
}

block_light_type world::get_light(const block_pos& pos)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return get_light(cpos, lpos);
}

std::optional<std::string_view> world::get_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state)
{
    auto id = get_block(cpos, lpos);

    if(id == BLOCK_ID_NULL) {
        return std::nullopt;
    }

    auto family = block_registry::find_family_of(id);

    if(family == nullptr) {
        return std::nullopt;
    }

    auto key = family->state_hash(state);
    auto decl_it = family->states.find(key);

    if(decl_it == family->states.cend()) {
        return std::nullopt; // this family has no such state
    }

    auto id_it = family->id_states.find(id);

    if(id_it == family->id_states.cend()) {
        return family->state_value(decl_it->second.default_value);
    }

    auto val_it = id_it->second.find(key);

    if(val_it == id_it->second.cend()) {
        return family->state_value(decl_it->second.default_value);
    }

    return family->state_value(val_it->second);
}

std::optional<std::string_view> world::get_state(const block_pos& pos, std::string_view state)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return get_state(cpos, lpos, state);
}

bool world::set_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state, std::string_view value)
{
    auto id = get_block(cpos, lpos);

    if(id == BLOCK_ID_NULL) {
        return false;
    }

    auto family = block_registry::find_family_of(id);

    if(family == nullptr) {
        return false;
    }

    auto key = family->state_hash(state);

    if(0 == family->states.count(key)) {
        return false;
    }

    emhash8::HashMap<blockstate_key_type, blockstate_val_type> map;
    auto id_it = family->id_states.find(id);

    if(id_it == family->id_states.cend()) {
        for(const auto& [decl_key, decl] : family->states) {
            map.try_emplace(decl_key, decl.default_value);
        }
    }
    else {
        map = id_it->second;
    }

    map.insert_or_assign(key, family->state_hash(value));

    auto new_id = block_registry::resolve_variant(family->stem_id, map);

    return set_block(cpos, lpos, new_id);
}

bool world::set_state(const block_pos& pos, std::string_view state, std::string_view value)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return set_state(cpos, lpos, state, value);
}

std::int32_t world::get_temperature_base(const chunk_pos& cpos, const local_pos& lpos)
{
    return 298; // TODO
}

std::int32_t world::get_temperature_base(const block_pos& pos)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return get_temperature_base(cpos, lpos);
}

std::int32_t world::get_temperature(const chunk_pos& cpos, const local_pos& lpos)
{
    return 298; // TODO
}

std::int32_t world::get_temperature(const block_pos& pos)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    return get_temperature(cpos, lpos);
}

void world::schedule(const chunk_pos& cpos, const local_pos& lpos, std::uint64_t deadline)
{
    if(auto chunk = find_chunk(cpos)) {
        auto index = utils::to_index(lpos);

        chunk->schedule(index, deadline);
    }
}

void world::schedule(const block_pos& pos, std::uint64_t deadline)
{
    auto cpos = utils::to_chunk(pos);
    auto lpos = utils::to_local(pos);
    schedule(cpos, lpos, deadline);
}

void world::shutdown(void)
{
    chunks.clear();
    chunk_entities.clear();
    basic_entities.clear();
    current_tick = 0;
}

void world::fixed_update(void)
{
    std::vector<std::size_t> due;

    chunk_entities.view<ChunkComponent>().each([&](ChunkComponent& component) {
        due.clear();

        component.ptr->pop_due(current_tick, due);

        for(auto index : due) {
            // TODO: dispatch on_stick for component.ptr->get_block(index) here
            // once there's a generic "call BlockCallback" helper
        }
    });

    current_tick += 1;
}
