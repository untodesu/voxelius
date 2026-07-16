#include "shared/pch.hh"

#include "shared/world/biome_lut.hh"

#include "core/exception.hh"
#include "core/utils/crc64.hh"

#include "shared/world/biome_registry.hh"

using TablePos = Eigen::Vector3i;

constexpr static std::size_t LUT_SIZE = 100;
constexpr static std::size_t LUT_TOTAL = LUT_SIZE * LUT_SIZE * LUT_SIZE;
constexpr static std::size_t LUT_COUNT = static_cast<std::size_t>(BIOME_REALM_COUNT);

using lookup_table_type = std::array<biome_id_type, LUT_TOTAL>;
using lookup_seeds_type = std::array<TablePos, LUT_TOTAL>;

static Eigen::AlignedBox3i s_bounds;
static std::array<lookup_table_type, LUT_COUNT> s_tables;
static std::array<TablePos, 26> s_neighbor_dirs;

static std::size_t table_index(const TablePos& pos)
{
    auto temp_idx = static_cast<std::size_t>(pos.x());
    auto humd_idx = static_cast<std::size_t>(pos.y());
    auto axis_idx = static_cast<std::size_t>(pos.z());
    return temp_idx * LUT_SIZE * LUT_SIZE + humd_idx * LUT_SIZE + axis_idx;
}

static void shift_seed(lookup_table_type& table, biome_id_type id, const TablePos& origin, std::mt19937& random)
{
    std::uniform_int_distribution<std::size_t> dir_dist(0, s_neighbor_dirs.size() - 1);

    auto direction = s_neighbor_dirs[dir_dist(random)];
    auto curr = origin;

    for(std::size_t i = 0; i < LUT_TOTAL; ++i) {
        TablePos next = curr + direction;

        if(!s_bounds.contains(next)) {
            direction = s_neighbor_dirs[dir_dist(random)];
            continue;
        }

        curr = next;

        auto index = table_index(curr);

        if(table[index] == BIOME_ID_NULL) {
            table[index] = id;
            return;
        }
    }

    LOG_WARNING("failed to resolve a nucleation conflict for biome {}", id);
}

static void place_seed(lookup_table_type& table, biome_id_type id, const TablePos& pos, std::mt19937& random)
{
    auto index = table_index(pos);
    auto occupied = table[index];

    if(occupied == BIOME_ID_NULL) {
        table[index] = id;
        return;
    }

    if(occupied == id) {
        return;
    }

    auto new_def = biome_registry::find_definition(id);
    auto old_def = biome_registry::find_definition(occupied);

    vx::throw_if(new_def == nullptr, "missing biome definition");
    vx::throw_if(old_def == nullptr, "missing biome definition");

    bool displace_old;

    if(new_def->priority > old_def->priority) {
        displace_old = true;
    }
    else if(new_def->priority < old_def->priority) {
        displace_old = false;
    }
    else {
        displace_old = std::uniform_int_distribution<int>(0, 1)(random);
    }

    if(displace_old) {
        table[index] = id;

        shift_seed(table, occupied, pos, random);
    }
    else {
        shift_seed(table, id, pos, random);
    }
}

static void table_init(biome_realm realm, lookup_table_type& table)
{
    table.fill(BIOME_ID_NULL);

    std::mt19937 random;
    random.seed(UINT32_C(0xB10BEE5E) ^ (static_cast<std::uint32_t>(realm) * UINT32_C(0x9E3779B9)));

    auto definitions = biome_registry::all_definitions();

    for(biome_id_type id = 1; id < definitions.size(); id += 1) {
        const auto& def = definitions[id];

        if(def.realm == realm) {
            TablePos pos(def.lut_temp, def.lut_humd, def.lut_axis);
            pos = pos.cwiseMin(LUT_SIZE - 1);
            pos = pos.cwiseMax(0);

            place_seed(table, id, pos, random);
        }
    }
}

static void seed_distance_field(const lookup_table_type& ids, lookup_seeds_type& seeds)
{
    for(int temp = 0; temp < static_cast<int>(LUT_SIZE); ++temp) {
        for(int humd = 0; humd < static_cast<int>(LUT_SIZE); ++humd) {
            for(int axis = 0; axis < static_cast<int>(LUT_SIZE); ++axis) {
                auto pos = TablePos(temp, humd, axis);
                auto index = table_index(pos);

                if(ids[index] == BIOME_ID_NULL) {
                    continue;
                }

                seeds[index] = pos;
            }
        }
    }
}

static std::pair<biome_id_type, TablePos> jfa_query(const TablePos& pos, int step, const lookup_table_type& ids,
    const lookup_seeds_type& seeds)
{
    auto best_id = BIOME_ID_NULL;
    auto best_seed = TablePos(TablePos::Zero());
    auto best_dist = std::numeric_limits<int>::max();

    for(int ox = -step; ox <= step; ox += step) {
        for(int oy = -step; oy <= step; oy += step) {
            for(int oz = -step; oz <= step; oz += step) {
                TablePos neighbour = pos + TablePos(ox, oy, oz);

                if(!s_bounds.contains(neighbour)) {
                    continue;
                }

                auto index = table_index(neighbour);
                auto candidate_id = ids[index];

                if(candidate_id == BIOME_ID_NULL) {
                    continue;
                }

                auto candidate_seed = seeds[index];
                auto dist = static_cast<int>((pos - candidate_seed).squaredNorm());

                if(dist < best_dist) {
                    best_dist = dist;
                    best_id = candidate_id;
                    best_seed = candidate_seed;
                }
            }
        }
    }

    return std::make_pair(best_id, best_seed);
}

static void jfa_step(int step, const lookup_table_type& read_ids, const lookup_seeds_type& read_seeds, lookup_table_type& write_ids,
    lookup_seeds_type& write_seeds, BS::light_thread_pool& threads)
{
    threads.detach_blocks(0, LUT_TOTAL, [&](std::size_t first, std::size_t last) {
        for(std::size_t index = first; index < last; index += 1) {
            TablePos pos;
            pos.x() = static_cast<int>(index / (LUT_SIZE * LUT_SIZE));
            pos.y() = static_cast<int>((index / LUT_SIZE) % LUT_SIZE);
            pos.z() = static_cast<int>(index % LUT_SIZE);

            auto jfa = jfa_query(pos, step, read_ids, read_seeds);

            write_ids[index] = jfa.first;
            write_seeds[index] = jfa.second;
        }
    });

    threads.wait();
}

static void table_fill(lookup_table_type& table, BS::light_thread_pool& threads)
{
    auto ids_a = std::make_unique<lookup_table_type>(table);
    auto ids_b = std::make_unique<lookup_table_type>();
    auto seeds_a = std::make_unique<lookup_seeds_type>();
    auto seeds_b = std::make_unique<lookup_seeds_type>();

    seed_distance_field(*ids_a, *seeds_a);

    auto read_ids = ids_a.get();
    auto read_seeds = seeds_a.get();

    auto write_ids = ids_b.get();
    auto write_seeds = seeds_b.get();

    auto step = static_cast<int>(LUT_SIZE) / 2;

    while(step >= 1) {
        jfa_step(step, *read_ids, *read_seeds, *write_ids, *write_seeds, threads);

        std::swap(read_ids, write_ids);
        std::swap(read_seeds, write_seeds);

        step >>= 1;
    }

    for(int extra_step : std::array { 2, 1, 1 }) {
        jfa_step(extra_step, *read_ids, *read_seeds, *write_ids, *write_seeds, threads);

        std::swap(read_ids, write_ids);
        std::swap(read_seeds, write_seeds);
    }

    table = std::move(read_ids[0]);
}

void biome_lut::generate(void)
{
    s_bounds = Eigen::AlignedBox3i(TablePos::Zero(), TablePos::Constant(static_cast<int>(LUT_SIZE) - 1));

    std::size_t i = 0;

    for(int dx = -1; dx <= 1; dx += 1) {
        for(int dy = -1; dy <= 1; dy += 1) {
            for(int dz = -1; dz <= 1; dz += 1) {
                if(dx == 0 && dy == 0 && dz == 0) {
                    continue;
                }

                s_neighbor_dirs[i++] = TablePos(dx, dy, dz);
            }
        }
    }

    table_init(BIOME_REALM_SURFACE, s_tables[BIOME_REALM_SURFACE]);
    table_init(BIOME_REALM_UNDERGROUND, s_tables[BIOME_REALM_UNDERGROUND]);
    table_init(BIOME_REALM_THE_DEPTHS, s_tables[BIOME_REALM_THE_DEPTHS]);
    table_init(BIOME_REALM_SKY, s_tables[BIOME_REALM_SKY]);

    BS::light_thread_pool threads;
    table_fill(s_tables[BIOME_REALM_SURFACE], threads);
    table_fill(s_tables[BIOME_REALM_UNDERGROUND], threads);
    table_fill(s_tables[BIOME_REALM_THE_DEPTHS], threads);
    table_fill(s_tables[BIOME_REALM_SKY], threads);
}

const BiomeDefinition* biome_lut::find(biome_realm realm, std::uint8_t temp, std::uint8_t humd, std::uint8_t axis)
{
    if(realm >= BIOME_REALM_COUNT) {
        return nullptr;
    }

    TablePos pos(temp, humd, axis);

    if(!s_bounds.contains(pos)) {
        return nullptr;
    }

    auto& table = s_tables[static_cast<std::size_t>(realm)];
    return biome_registry::find_definition(table[table_index(pos)]);
}
