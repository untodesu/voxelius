#include "shared/pch.hh"

#include "shared/entity/required_class.hh"

#include "core/exception.hh"

#include "shared/entity/class_registry.hh"
#include "shared/mod_context.hh"

class_id_type required_class::player = CLASS_ID_NULL;

static std::optional<Identifier> s_pending_player;

static void set_pending(std::optional<Identifier>& pending, Identifier id, const ModContext* ctx, std::string_view name)
{
    if(pending.has_value()) {
        LOG_WARNING("{} overrides existing required {} class {} with {}", ctx->name_space(), name, pending->full_string(),
            id.full_string());
        LOG_WARNING("this is probably not a good idea!");
    }

    pending = id;
}

static void resolve_pending(std::optional<Identifier>& pending, class_id_type& resolved, std::string_view name)
{
    vx::throw_if_not_fmt(pending.has_value(), "missing required {} class", name);
    resolved = class_registry::find(pending.value());
    vx::throw_if_fmt(resolved == CLASS_ID_NULL, "required {} class {} not found", name, pending->full_string());
}

void required_class::set_player(const Identifier& id, const ModContext* ctx)
{
    set_pending(s_pending_player, id, ctx, "player");
}

void required_class::resolve(void)
{
    resolve_pending(s_pending_player, player, "player");
}
