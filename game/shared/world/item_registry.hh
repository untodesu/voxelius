#ifndef SHARED_ITEM_REGISTRY_HH
#define SHARED_ITEM_REGISTRY_HH 1
#pragma once

#include "core/resource/resource.hh"

#include "shared/types.hh"

// This resource is only defined client-side and
// resource_ptr<TextureGUI> should remain set to null
// anywhere else in the shared and server code
struct TextureGUI;

namespace world
{
struct ItemInfo final {
    std::string name;
    std::string texture;
    voxel_id place_voxel;

    resource_ptr<TextureGUI> cached_texture; // Client-side only
};
} // namespace world

namespace world
{
class ItemInfoBuilder final {
public:
    explicit ItemInfoBuilder(const char* name);
    virtual ~ItemInfoBuilder(void) = default;

public:
    ItemInfoBuilder& set_texture(const char* texture);
    ItemInfoBuilder& set_place_voxel(voxel_id place_voxel);

public:
    item_id build(void) const;

private:
    ItemInfo prototype;
};
} // namespace world

namespace world::item_registry
{
extern std::unordered_map<std::string, ItemInfoBuilder> builders;
extern std::unordered_map<std::string, item_id> names;
extern std::vector<std::shared_ptr<ItemInfo>> items;
} // namespace world::item_registry

namespace world::item_registry
{
ItemInfoBuilder& construct(const char* name);
ItemInfo* find(const char* name);
ItemInfo* find(const item_id item);
} // namespace world::item_registry

namespace world::item_registry
{
void purge(void);
} // namespace world::item_registry

namespace world::item_registry
{
std::uint64_t calcualte_checksum(void);
} // namespace world::item_registry

#endif // SHARED_ITEM_REGISTRY_HH
