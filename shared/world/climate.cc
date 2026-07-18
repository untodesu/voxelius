#include "shared/pch.hh"

#include "shared/world/climate.hh"

#include "shared/world/biome_registry.hh"

constexpr static int CLIMATE_SIZE = 100;
constexpr static std::size_t REALM_COUNT = static_cast<std::size_t>(BIOME_REALM_COUNT);
constexpr static std::size_t MAX_NUDGE_STEPS = 1000000;

struct ClimateSeed final {
    Eigen::Vector4i target;
    biome_id_type id;
    unsigned priority;
    float offset;
};

static std::array<std::vector<ClimateSeed>, REALM_COUNT> s_seeds;
static std::array<Eigen::Vector4i, 80> s_neighbour_dirs;
static std::once_flag s_neighbour_dirs_once;

static void build_neighbour_dirs(void)
{
    constexpr static int START = -1;
    constexpr static int END = 1;

    std::size_t index = 0;

    for(int i = START; i <= END; ++i) {
        for(auto j = START; j <= END; ++j) {
            for(auto k = START; k <= END; ++k) {
                for(auto l = START; l <= END; ++l) {
                    if(i == 0 && j == 0 && k == 0 && l == 0) {
                        continue;
                    }

                    s_neighbour_dirs[index] = Eigen::Vector4i(i, j, k, l);

                    index += 1;
                }
            }
        }
    }
}

static std::uint32_t pack_pos(const Eigen::Vector4i& pos)
{
    std::uint32_t result = 0;
    result |= static_cast<std::uint32_t>(pos.x()) & 0xFF;
    result |= (static_cast<std::uint32_t>(pos.y()) & 0xFF) << 8;
    result |= (static_cast<std::uint32_t>(pos.z()) & 0xFF) << 16;
    result |= (static_cast<std::uint32_t>(pos.w()) & 0xFF) << 24;
    return result;
}

static Eigen::Vector4i clamp_pos(Eigen::Vector4i pos)
{
    pos = pos.cwiseMax(Eigen::Vector4i::Zero());
    pos = pos.cwiseMin(Eigen::Vector4i::Constant(CLIMATE_SIZE - 1));
    return pos;
}

// static bool in_bounds(const ClimatePos& pos)
// {
//     return (pos.array() >= 0).all() && (pos.array() < CLIMATE_SIZE).all();
// }

// static float normalize_01(float sample)
// {
//     return std::clamp((sample + 1.0f) * 0.5f, 0.0f, 1.0f);
// }

static Eigen::Vector4f target_to_unit(const Eigen::Vector4i& pos)
{
    constexpr float scale = 1.0f / static_cast<float>(CLIMATE_SIZE - 1);

    Eigen::Vector4f result;
    result.x() = scale * static_cast<float>(pos.x());
    result.y() = scale * static_cast<float>(pos.y());
    result.z() = scale * static_cast<float>(pos.z());
    result.w() = scale * static_cast<float>(pos.w());

    return result;
}

static Eigen::Vector4f sample_to_unit(const ClimateSample& sample)
{
    Eigen::Vector4f result;
    result.x() = std::clamp((sample.temperature + 1.0f) * 0.5f, 0.0f, 1.0f);
    result.y() = std::clamp((sample.humidity + 1.0f) * 0.5f, 0.0f, 1.0f);
    result.z() = std::clamp((sample.continentalness + 1.0f) * 0.5f, 0.0f, 1.0f);
    result.w() = std::clamp((sample.weirdness + 1.0f) * 0.5f, 0.0f, 1.0f);
    return result;
}

static bool nudge(emhash8::HashMap<std::uint32_t, biome_id_type>& occupied, Eigen::Vector4i& pos, biome_id_type id, std::mt19937& random)
{
    static const Eigen::AlignedBox4i bounds(Eigen::Vector4i::Zero(), Eigen::Vector4i::Constant(CLIMATE_SIZE - 1));

    std::call_once(s_neighbour_dirs_once, &build_neighbour_dirs);

    auto key = pack_pos(pos);

    if(0 == occupied.count(key)) {
        occupied.emplace(key, id);
        return true;
    }

    std::uniform_int_distribution<std::size_t> dir_dist(0, s_neighbour_dirs.size() - 1);
    auto direction = s_neighbour_dirs.at(dir_dist(random));
    auto current = pos;

    for(std::size_t step = 0; step < MAX_NUDGE_STEPS; step += 1) {
        Eigen::Vector4i next = current + direction;

        if(!bounds.contains(next)) {
            direction = s_neighbour_dirs.at(dir_dist(random));
            continue;
        }

        current = next;
        key = pack_pos(current);

        if(0 == occupied.count(key)) {
            occupied.emplace(key, id);
            pos = current;
            return true;
        }
    }

    return false;
}

static void rebuild_realm(biome_realm realm)
{
    auto& out = s_seeds.at(static_cast<std::size_t>(realm));
    auto definitions = biome_registry::all_definitions();

    out.clear();
    out.reserve(definitions.size());

    std::mt19937 random;
    random.seed(UINT32_C(0xB10BEE5E) ^ (static_cast<std::uint32_t>(realm) * UINT32_C(0x9E3779B9)));

    for(biome_id_type id = 1; id < definitions.size(); id += 1) {
        const auto& def = definitions[id];

        if(def.realm == realm) {
            ClimateSeed seed {};
            seed.id = id;
            seed.priority = def.priority;
            seed.offset = def.offset;
            seed.target = clamp_pos(Eigen::Vector4i(def.temperature, def.humidity, def.continentalness, def.weirdness));
            out.push_back(seed);
        }
    }

    std::stable_sort(out.begin(), out.end(), [](const ClimateSeed& a, const ClimateSeed& b) {
        return a.priority > b.priority;
    });

    emhash8::HashMap<std::uint32_t, biome_id_type> occupied;

    for(auto& seed : out) {
        if(!nudge(occupied, seed.target, seed.id, random)) {
            LOG_WARNING("failed to place biome {} uniquely in realm {}", seed.id, static_cast<unsigned>(realm));
        }
    }
}

void climate::rebuild(void)
{
    std::call_once(s_neighbour_dirs_once, &build_neighbour_dirs);

    for(unsigned realm = 0; realm < BIOME_REALM_COUNT; realm += 1) {
        rebuild_realm(static_cast<biome_realm>(realm));
    }
}

const BiomeDefinition* climate::find(biome_realm realm, const ClimateSample& sample)
{
    if(realm >= BIOME_REALM_COUNT) {
        return nullptr; // WTF
    }

    const auto& seeds = s_seeds.at(static_cast<std::size_t>(realm));

    if(seeds.empty()) {
        return nullptr;
    }

    auto unit = sample_to_unit(sample);
    auto axis_weights = Eigen::Vector4f(1.0f, 1.0f, 0.75f, 1.5f);

    auto best_id = BIOME_ID_NULL;
    auto best_score = std::numeric_limits<float>::max();
    auto best_priority = 0U;

    for(const auto& seed : seeds) {
        auto target = target_to_unit(seed.target);
        auto delta = Eigen::Vector4f((unit - target).cwiseProduct(axis_weights));
        auto dist_squared = delta.squaredNorm();
        auto score = dist_squared + seed.offset;

        if(score < best_score || (score == best_score && seed.priority > best_priority)) {
            best_score = score;
            best_priority = seed.priority;
            best_id = seed.id;
        }
    }

    return biome_registry::find_definition(best_id);
}
