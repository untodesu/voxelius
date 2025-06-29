#ifndef SHARED_ITEM_REGISTRY_HH
#define SHARED_ITEM_REGISTRY_HH 1
#pragma once

#include "core/resource.hh"

#include "shared/types.hh"

// This resource is only defined client-side and
// resource_ptr<TextureGUI> should remain set to null
// anywhere else in the shared and server code
struct TextureGUI;

struct ItemInfo final {
    std::string name;
    std::string texture;
    voxel_id place_voxel;

    resource_ptr<TextureGUI> cached_texture; // Client-side only
};

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

namespace item_registry
{
extern std::unordered_map<std::string, ItemInfoBuilder> builders;
extern std::unordered_map<std::string, item_id> names;
extern std::vector<std::shared_ptr<ItemInfo>> items;
} // namespace item_registry

namespace item_registry
{
ItemInfoBuilder& construct(const char* name);
ItemInfo* find(const char* name);
ItemInfo* find(const item_id item);
} // namespace item_registry

namespace item_registry
{
void purge(void);
} // namespace item_registry

namespace item_registry
{
std::uint64_t calcualte_checksum(void);
} // namespace item_registry

#endif // SHARED_ITEM_REGISTRY_HH
