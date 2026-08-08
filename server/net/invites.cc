#include "server/pch.hh"

#include "server/net/invites.hh"

#include "server/net/whitelist.hh"

static emhash8::HashMap<std::uint64_t, std::monostate> s_invites;
static std::mt19937_64 s_randomizer;

void invites::init(void)
{
    std::random_device noise;
    s_randomizer.seed(noise());
}

void invites::shutdown(void)
{
    s_invites.clear();
}

std::uint64_t invites::issue(void)
{
    std::uint64_t token = s_randomizer();

    while(s_invites.contains(token)) {
        token = s_randomizer();
    }

    s_invites.insert_or_assign(token, std::monostate {});
    return token;
}

bool invites::consume(std::uint64_t token, const ed25519::pkey_type& pkey)
{
    if(s_invites.contains(token)) {
        s_invites.erase(token);
        whitelist::add(pkey);
        return true;
    }

    return false;
}
