#include "client/pch.hh"

#include "client/block_models.hh"

static std::vector<std::unique_ptr<BakedBlockModel>> s_models;

void block_models::init_late(void)
{
}

void block_models::shutdown(void)
{
    s_models.clear();
}

const BakedBlockModel* block_models::find(block_id_type id) noexcept
{
    if(id == BLOCK_ID_NULL || id >= s_models.size()) {
        return nullptr;
    }

    return s_models[id].get();
}
