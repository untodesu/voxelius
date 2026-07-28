#include "shared/pch.hh"

#include "shared/world/climate.hh"

#include "shared/world/biome_registry.hh"

constexpr static int CLIMATE_SIZE = 100;
constexpr static std::size_t CLIMATE_DIMS = 5;
constexpr static std::size_t REALM_COUNT = static_cast<std::size_t>(BIOME_REALM_COUNT);
constexpr static std::size_t MAX_NUDGE_STEPS = 1000000;

using ClimatePos = std::array<std::uint8_t, CLIMATE_DIMS>;
using ClimateDir = std::array<int, CLIMATE_DIMS>;

struct ClimateSeed final {
    ClimatePos target;
    biome_id_type id;
    unsigned priority;
    float offset;
};

static std::array<std::vector<ClimateSeed>, REALM_COUNT> s_seeds;
static std::array<ClimateDir, 242> s_neighbour_dirs;
static std::once_flag s_neighbour_dirs_once;

static void build_neighbour_dirs(void)
{
    constexpr static int START = -1;
    constexpr static int END = 1;

    std::size_t index = 0;

    for(int a = START; a <= END; ++a) {
        for(int b = START; b <= END; ++b) {
            for(int c = START; c <= END; ++c) {
                for(int d = START; d <= END; ++d) {
                    for(int e = START; e <= END; ++e) {
                        if(a == 0 && b == 0 && c == 0 && d == 0 && e == 0) {
                            continue;
                        }

                        ClimateDir dir {};
                        dir[0] = a;
                        dir[1] = b;
                        dir[2] = c;
                        dir[3] = d;
                        dir[4] = e;

                        s_neighbour_dirs[index] = std::move(dir);

                        index += 1;
                    }
                }
            }
        }
    }
}

static std::uint64_t pack_pos(const ClimatePos& pos)
{
    std::uint64_t result = 0;
    result |= static_cast<std::uint64_t>(pos[0]) & 0x7F;
    result |= (static_cast<std::uint64_t>(pos[1]) & 0x7F) << 7;
    result |= (static_cast<std::uint64_t>(pos[2]) & 0x7F) << 14;
    result |= (static_cast<std::uint64_t>(pos[3]) & 0x7F) << 21;
    result |= (static_cast<std::uint64_t>(pos[4]) & 0x7F) << 28;
    return result;
}

static ClimatePos clamp_pos(ClimatePos pos)
{
    pos[0] = static_cast<std::uint8_t>(std::clamp<int>(pos[0], 0, CLIMATE_SIZE - 1));
    pos[1] = static_cast<std::uint8_t>(std::clamp<int>(pos[1], 0, CLIMATE_SIZE - 1));
    pos[2] = static_cast<std::uint8_t>(std::clamp<int>(pos[2], 0, CLIMATE_SIZE - 1));
    pos[3] = static_cast<std::uint8_t>(std::clamp<int>(pos[3], 0, CLIMATE_SIZE - 1));
    pos[4] = static_cast<std::uint8_t>(std::clamp<int>(pos[4], 0, CLIMATE_SIZE - 1));
    return pos;
}

static std::array<float, CLIMATE_DIMS> target_to_unit(const ClimatePos& pos)
{
    constexpr static auto scale = 1.0f / static_cast<float>(CLIMATE_SIZE - 1);

    std::array<float, CLIMATE_DIMS> result {};
    result[0] = scale * static_cast<float>(pos[0]);
    result[1] = scale * static_cast<float>(pos[1]);
    result[2] = scale * static_cast<float>(pos[2]);
    result[3] = scale * static_cast<float>(pos[3]);
    result[4] = scale * static_cast<float>(pos[4]);
    return result;
}

static std::array<float, CLIMATE_DIMS> sample_to_unit(const ClimateSample& sample)
{
    return {
        climate::normalize_01(sample.temperature),
        climate::normalize_01(sample.humidity),
        climate::normalize_01(sample.continentalness),
        climate::normalize_01(sample.erosion),
        climate::normalize_01(sample.weirdness),
    };
}

static bool nudge(emhash8::HashMap<std::uint64_t, biome_id_type>& occupied, ClimatePos& pos, biome_id_type id, std::mt19937& random)
{
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
        ClimatePos next {};

        while(true) {
            auto in_bounds = true;

            for(std::size_t i = 0; i < CLIMATE_DIMS; ++i) {
                auto value = static_cast<int>(current[i]) + direction[i];

                if(value < 0 || value >= CLIMATE_SIZE) {
                    in_bounds = false;
                    break;
                }

                next[i] = static_cast<std::uint8_t>(value);
            }

            if(in_bounds) {
                break;
            }

            direction = s_neighbour_dirs.at(dir_dist(random));
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
            ClimatePos target {};
            target[0] = def.temperature;
            target[1] = def.humidity;
            target[2] = def.continentalness;
            target[3] = def.erosion;
            target[4] = def.weirdness;

            ClimateSeed seed {};
            seed.id = id;
            seed.priority = def.priority;
            seed.offset = def.offset;
            seed.target = clamp_pos(std::move(target));

            out.emplace_back(std::move(seed));
        }
    }

    std::stable_sort(out.begin(), out.end(), [](const ClimateSeed& a, const ClimateSeed& b) {
        return a.priority > b.priority;
    });

    emhash8::HashMap<std::uint64_t, biome_id_type> occupied;

    for(auto& seed : out) {
        if(!nudge(occupied, seed.target, seed.id, random)) {
            LOG_WARNING("failed to place biome {} uniquely in realm {}", seed.id, static_cast<unsigned>(realm));
        }
    }
}

float climate::peaks_valleys(float weirdness)
{
    auto w = std::abs(weirdness);
    return 1.0f - std::abs(3.0f * w - 2.0f);
}

float climate::normalize_01(float sample)
{
    return std::clamp((sample + 1.0f) * 0.5f, 0.0f, 1.0f);
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
    constexpr std::array<float, CLIMATE_DIMS> axis_weights = { 1.0f, 1.0f, 0.75f, 1.0f, 1.5f };

    auto best_id = BIOME_ID_NULL;
    auto best_score = std::numeric_limits<float>::max();
    auto best_priority = 0U;

    for(const auto& seed : seeds) {
        auto target = target_to_unit(seed.target);
        auto dist_squared = 0.0f;

        for(std::size_t i = 0; i < CLIMATE_DIMS; ++i) {
            auto delta = unit[i] - target[i];
            delta *= axis_weights[i];

            dist_squared += delta * delta;
        }

        auto score = dist_squared + seed.offset;

        if(score < best_score || (score == best_score && seed.priority > best_priority)) {
            best_score = score;
            best_priority = seed.priority;
            best_id = seed.id;
        }
    }

    return biome_registry::find_definition(best_id);
}
