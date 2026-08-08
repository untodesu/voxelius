#include "shared/pch.hh"

#include "shared/entity/class_registry.hh"

#include "shared/mod_context.hh"

static std::vector<ClassDefinition> s_definitions;
static emhash8::HashMap<Identifier, class_id_type> s_names;
static emhash8::HashMap<class_id_type, Identifier> s_reverse_names;
static std::uint64_t s_checksum;

static void update_checksum(void)
{
    // TODO: go through each class definition and compute a checksum based on its contents
}

std::span<const ClassDefinition> class_registry::all_definitions(void)
{
    return s_definitions;
}

std::uint64_t class_registry::checksum(void)
{
    return s_checksum;
}

void class_registry::commit(ModContext& ctx)
{
    if(s_definitions.empty()) {
        s_definitions.emplace_back();
    }

    auto classes = ctx.take_classes();
    auto names = ctx.take_class_names();

    class_id_type class_offset;

    if(classes.empty()) {
        class_offset = static_cast<class_id_type>(s_definitions.size());
    }
    else {
        class_offset = static_cast<class_id_type>(s_definitions.size()) - 1;
    }

    for(const auto& [name, local_id] : names) {
        auto global_id = local_id + class_offset;
        auto [it, inserted] = s_names.try_emplace(name, global_id);

        if(!inserted) {
            LOG_WARNING("duplicate class name: {}", name.full_string());
            continue;
        }

        s_reverse_names.try_emplace(global_id, name);
    }

    if(classes.size()) {
        s_definitions.insert(s_definitions.end(), std::make_move_iterator(classes.begin() + 1), std::make_move_iterator(classes.end()));
    }
}

void class_registry::purge(void)
{
    s_definitions.clear();
    s_names.clear();
    s_reverse_names.clear();
    s_checksum = 0;
}

class_id_type class_registry::find(const Identifier& id)
{
    auto it = s_names.find(id);

    if(it == s_names.cend())
        return CLASS_ID_NULL;
    return it->second;
}

std::optional<Identifier> class_registry::name_of(class_id_type id)
{
    auto it = s_reverse_names.find(id);

    if(it == s_reverse_names.cend()) {
        return std::nullopt;
    }

    return it->second;
}

const ClassDefinition* class_registry::find_definition(class_id_type id)
{
    if(id == CLASS_ID_NULL || id >= s_definitions.size())
        return nullptr;
    return &s_definitions[id];
}

const ClassDefinition* class_registry::find_definition(const Identifier& id)
{
    return find_definition(find(id));
}
