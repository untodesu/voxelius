#ifndef D1058B64_4176_4F3F_8A52_75E0BAEF0668
#define D1058B64_4176_4F3F_8A52_75E0BAEF0668

#include "core/cvar/bits.hh"

#include "shared/net/packet.hh"

struct packet::Cvar_ServerVars final : public packet::Base<packet::CVAR_SERVER_VARS> {
    struct Variable final {
        std::string name;
        std::string value;
        cvar::flag_bits flags;
        std::string help;
    };

    static void encode(const Cvar_ServerVars& packet, WriteBuffer& buffer);
    static void decode(Cvar_ServerVars& packet, ReadBuffer& buffer);

    static void build_full(Cvar_ServerVars& packet);
    static void build_dirty(std::span<const std::string> names, Cvar_ServerVars& packet);

    bool full;
    std::vector<Variable> variables;
};

#endif /* D1058B64_4176_4F3F_8A52_75E0BAEF0668 */
