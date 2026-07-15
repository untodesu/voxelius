#include "shared/pch.hh"

#include "shared/biome_lut.hh"

#include "shared/biome_registry.hh"

constexpr static std::size_t LUT_WIDTH_T = static_cast<std::size_t>(BIOME_TEMP_MAX - BIOME_TEMP_MIN + 1);
constexpr static std::size_t LUT_WIDTH_H = static_cast<std::size_t>(BIOME_HUMID_MAX - BIOME_HUMID_MIN + 1);
constexpr static std::size_t LUT_WIDTH_A = static_cast<std::size_t>(BIOME_AXIS_MAX - BIOME_AXIS_MIN + 1);
constexpr static std::size_t LUT_VOLUME = LUT_WIDTH_T * LUT_WIDTH_H * LUT_WIDTH_A;

constexpr static unsigned NUM_REALMS = static_cast<unsigned>(BIOME_REALM__COUNT);

constexpr static std::array OFFSETS = {
    std::array { -1, 0, 0 },
    std::array { +1, 0, 0 },
    std::array { 0, -1, 0 },
    std::array { 0, +1, 0 },
    std::array { 0, 0, -1 },
    std::array { 0, 0, +1 },
};

static std::vector<biome_id_type> s_tables[NUM_REALMS];

static std::size_t lut_index(int lt, int lh, int la)
{
    auto lt_sz = static_cast<std::size_t>(lt);
    auto lh_sz = static_cast<std::size_t>(lh);
    auto la_sz = static_cast<std::size_t>(la);
    return (lt_sz * LUT_WIDTH_H + lh_sz) * LUT_WIDTH_A + la_sz;
}

static int biome_priority(biome_id_type id)
{
    if(auto def = biome_registry::find_definition(id)) {
        return def->priority;
    }

    return std::numeric_limits<int>::min();
}

static std::vector<biome_id_type> build_table(biome_realm realm)
{
    std::vector<biome_id_type> table;
    table.assign(LUT_VOLUME, BIOME_ID_NULL);

    std::vector<int> distance(LUT_VOLUME, -1);
    std::vector<std::size_t> frontier;

    auto definitions = biome_registry::all_definitions();

    for(std::size_t i = 1; i < definitions.size(); ++i) {
        const auto& def = definitions[i];

        if(def.realm != realm) {
            continue;
        }

        auto lt = std::clamp(def.temperature, BIOME_TEMP_MIN, BIOME_TEMP_MAX) - BIOME_TEMP_MIN;
        auto lh = std::clamp(def.humidity, BIOME_HUMID_MIN, BIOME_HUMID_MAX) - BIOME_HUMID_MIN;
        auto la = std::clamp(def.extra_axis, BIOME_AXIS_MIN, BIOME_AXIS_MAX) - BIOME_AXIS_MIN;

        auto index = lut_index(lt, lh, la);
        auto biome_id = static_cast<biome_id_type>(i);

        if(distance[index] < 0) {
            distance[index] = 0;
            table[index] = biome_id;
            frontier.push_back(index);
        }
        else if(distance[index] == 0) {
            if(biome_priority(biome_id) > biome_priority(table[index])) {
                table[index] = biome_id;
            }
        }
    }

    if(frontier.empty()) {
        return table;
    }

    std::vector<std::size_t> next_frontier;
    next_frontier.reserve(frontier.size());

    while(frontier.size()) {
        next_frontier.clear();

        for(auto cell : frontier) {
            auto lt = static_cast<int>(cell / (LUT_WIDTH_H * LUT_WIDTH_A));
            auto rem = cell % (LUT_WIDTH_H * LUT_WIDTH_A);

            auto lh = static_cast<int>(rem / LUT_WIDTH_A);
            auto la = static_cast<int>(rem % LUT_WIDTH_A);

            auto next_dist = distance[cell] + 1;

            for(const auto& offset : OFFSETS) {
                auto nt = lt + offset[0];
                auto nh = lh + offset[1];
                auto na = la + offset[2];

                auto skip = false;
                skip = skip || nt < 0 || nt >= static_cast<int>(LUT_WIDTH_T);
                skip = skip || nh < 0 || nh >= static_cast<int>(LUT_WIDTH_H);
                skip = skip || na < 0 || na >= static_cast<int>(LUT_WIDTH_A);

                if(skip) {
                    continue;
                }

                auto idx = lut_index(nt, nh, na);

                if(distance[idx] < 0) {
                    distance[idx] = next_dist;
                    table[idx] = table[cell];
                    next_frontier.push_back(idx);
                }
                else if(distance[idx] == next_dist) {
                    if(biome_priority(table[cell]) > biome_priority(table[idx])) {
                        table[idx] = table[cell];
                    }
                }
            }
        }

        std::swap(frontier, next_frontier);
    }

    return table;
}

void biome_lut::rebuild(void)
{
    s_tables[BIOME_REALM_SURFACE] = build_table(BIOME_REALM_SURFACE);
    s_tables[BIOME_REALM_CAVE] = build_table(BIOME_REALM_CAVE);
    s_tables[BIOME_REALM_DEEP] = build_table(BIOME_REALM_DEEP);
    s_tables[BIOME_REALM_SKY] = build_table(BIOME_REALM_SKY);
}

void biome_lut::purge(void)
{
    for(auto& table : s_tables) {
        table.clear();
    }
}

biome_id_type biome_lut::lookup(biome_realm realm, int temperature, int humidity, int axis)
{
    if(realm >= NUM_REALMS) {
        return BIOME_ID_NULL;
    }

    const auto& table = s_tables[realm];

    if(table.empty()) {
        return BIOME_ID_NULL;
    }

    auto lt = std::clamp(temperature, BIOME_TEMP_MIN, BIOME_TEMP_MAX) - BIOME_TEMP_MIN;
    auto lh = std::clamp(humidity, BIOME_HUMID_MIN, BIOME_HUMID_MAX) - BIOME_HUMID_MIN;
    auto la = std::clamp(axis, BIOME_AXIS_MIN, BIOME_AXIS_MAX) - BIOME_AXIS_MIN;
    auto index = lut_index(lt, lh, la);

    return table[index];
}
