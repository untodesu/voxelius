#include "server/pch.hh"

#include "server/world/universe.hh"

#include "core/config/number.hh"
#include "core/config/string.hh"
#include "core/io/buffer.hh"
#include "core/io/config_map.hh"
#include "core/utils/epoch.hh"

#include "shared/world/chunk.hh"
#include "shared/world/dimension.hh"

#include "server/world/inhabited.hh"
#include "server/world/overworld.hh"

#include "server/globals.hh"

struct DimensionMetadata final {
    std::string config_path;
    std::string zvox_dir;
    io::ConfigMap config;
};

static config::String universe_name("save");

static io::ConfigMap universe_config;
static config::Unsigned64 universe_config_seed;
static config::String universe_spawn_dimension("world");

static std::string universe_config_path;
static std::unordered_map<world::Dimension*, DimensionMetadata*> metadata_map;

static std::string make_chunk_filename(const DimensionMetadata* metadata, const chunk_pos& cpos)
{
    const auto unsigned_x = static_cast<std::uint32_t>(cpos.x);
    const auto unsigned_y = static_cast<std::uint32_t>(cpos.y);
    const auto unsigned_z = static_cast<std::uint32_t>(cpos.z);
    return std::format("{}/{:08X}-{:08X}-{:08X}.zvox", metadata->zvox_dir, unsigned_x, unsigned_y, unsigned_z);
}

static void add_new_dimension(world::Dimension* dimension)
{
    if(globals::dimensions.count(std::string(dimension->get_name()))) {
        spdlog::critical("universe: dimension named {} already exists", dimension->get_name());
        std::terminate();
    }

    auto dimension_dir = std::format("{}/{}", universe_name.get(), dimension->get_name());

    if(!PHYSFS_mkdir(dimension_dir.c_str())) {
        spdlog::critical("universe: {}: {}", dimension_dir, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    auto metadata = new DimensionMetadata;
    metadata->config_path = std::format("{}/dimension.conf", dimension_dir);
    metadata->zvox_dir = std::format("{}/chunk", dimension_dir);

    if(!PHYSFS_mkdir(metadata->zvox_dir.c_str())) {
        spdlog::critical("universe: {}: {}", metadata->zvox_dir, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    globals::dimensions.insert_or_assign(std::string(dimension->get_name()), dimension);

    auto& mapped_metadata = metadata_map.insert_or_assign(dimension, metadata).first->second;

    dimension->init(mapped_metadata->config);

    mapped_metadata->config.load_file(mapped_metadata->config_path.c_str());

    dimension->init_late(universe_config_seed.get_value());
}

static void internal_save_chunk(
    const DimensionMetadata* metadata, const world::Dimension* dimension, const chunk_pos& cpos, const world::Chunk* chunk)
{
    auto path = make_chunk_filename(metadata, cpos);

    io::WriteBuffer buffer;
    chunk->get_voxels().serialize(buffer);

    if(auto file = buffer.to_file(path.c_str())) {
        PHYSFS_close(file);
        return;
    }
}

void world::universe::init(void)
{
    // If the world is newly created, the seed will
    // be chosed based on the current system's view on UNIX time
    universe_config_seed.set_value(utils::unix_microseconds());

    // We're going to read files from directory named with
    // the value of this config value. Since config is also
    // read from command line, the [--universe <name>] parameter still works
    globals::server_config.add_value("universe", universe_name);

    universe_config.add_value("global_seed", universe_config_seed);
    universe_config.add_value("spawn_dimension", universe_spawn_dimension);
}

void world::universe::init_late(void)
{
    const auto universe_dir = std::string(universe_name.get());

    if(!PHYSFS_mkdir(universe_dir.c_str())) {
        spdlog::critical("universe: {}: {}", universe_dir, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    universe_config_path = std::format("{}/universe.conf", universe_dir);
    universe_config.load_file(universe_config_path.c_str());

    add_new_dimension(new Overworld("world"));

    // UNDONE: lua scripts to setup dimensions
    if(globals::dimensions.empty()) {
        spdlog::critical("universe: no dimensions");
        std::terminate();
    }

    auto spawn_dimension = globals::dimensions.find(universe_spawn_dimension.get_value());

    if(spawn_dimension == globals::dimensions.cend()) {
        spdlog::critical("universe: {} is not a valid dimension name", universe_spawn_dimension.get());
        std::terminate();
    }

    globals::spawn_dimension = spawn_dimension->second;
}

void world::universe::shutdown(void)
{
    for(const auto metadata : metadata_map) {
        metadata.second->config.save_file(metadata.second->config_path.c_str());
        delete metadata.second;
    }

    metadata_map.clear();

    for(const auto dimension : globals::dimensions) {
        world::universe::save_all_chunks(dimension.second);
        delete dimension.second;
    }

    globals::dimensions.clear();
    globals::spawn_dimension = nullptr;

    universe_config.save_file(universe_config_path.c_str());
}

world::Chunk* world::universe::load_chunk(Dimension* dimension, const chunk_pos& cpos)
{
    if(auto chunk = dimension->find_chunk(cpos)) {
        // Just return the existing chunk which is
        // most probable to be up to date compared to
        // whatever the hell is currently stored on disk
        return chunk;
    }

    auto metadata = metadata_map.find(dimension);

    if(metadata == metadata_map.cend()) {
        // The dimension is for sure a weird one
        return nullptr;
    }

    if(auto file = PHYSFS_openRead(make_chunk_filename(metadata->second, cpos).c_str())) {
        VoxelStorage voxels;
        io::ReadBuffer buffer(file);
        voxels.deserialize(buffer);

        PHYSFS_close(file);

        auto chunk = dimension->create_chunk(cpos);
        chunk->set_voxels(voxels);

        // Make sure we're going to save it later
        dimension->chunks.emplace_or_replace<Inhabited>(chunk->get_entity());

        return chunk;
    }

    return nullptr;
}

void world::universe::save_chunk(Dimension* dimension, const chunk_pos& cpos)
{
    auto metadata = metadata_map.find(dimension);

    if(metadata == metadata_map.cend()) {
        // Cannot save a chunk in a dimension
        // that doesn't have a metadata struct
        return;
    }

    if(auto chunk = dimension->find_chunk(cpos)) {
        internal_save_chunk(metadata->second, dimension, cpos, chunk);
    }
}

void world::universe::save_all_chunks(Dimension* dimension)
{
    auto group = dimension->chunks.group(entt::get<ChunkComponent, Inhabited>);
    auto metadata = metadata_map.find(dimension);

    if(metadata == metadata_map.cend()) {
        // Cannot save a chunk in a dimension
        // that doesn't have a metadata struct
        return;
    }

    for(auto [entity, chunk] : group.each()) {
        internal_save_chunk(metadata->second, dimension, chunk.cpos, chunk.chunk);
    }
}
