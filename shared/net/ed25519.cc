#include "shared/pch.hh"

#include "shared/net/ed25519.hh"

template<std::size_t N>
static std::optional<std::array<std::byte, N>> import_xstr(std::string_view xstr)
{
    if(2 * N == xstr.size()) {
        std::array<std::byte, N> result;

        for(std::size_t i = 0; i < N; ++i) {
            if(1 == std::sscanf(&xstr[2 * i], "%2hhX", reinterpret_cast<unsigned char*>(&result[i])))
                continue;
            return std::nullopt;
        }

        return result;
    }

    return std::nullopt;
}

template<std::size_t N>
static std::string export_xstr(std::span<const std::byte, N> buffer)
{
    std::string xstr;
    xstr.resize(2 * N);

    for(std::size_t i = 0; i < N; ++i) {
        std::snprintf(&xstr[2 * i], 3, "%02hhx", static_cast<unsigned char>(buffer[i]));
    }

    return xstr;
}

ed25519::seed_type ed25519::generate_seed(void)
{
    seed_type result;
    ed25519_create_seed(reinterpret_cast<unsigned char*>(result.data()));
    return result;
}

ed25519::pair_type ed25519::generate_pair(const seed_type& seed)
{
    pair_type result;

    auto seed_ptr = reinterpret_cast<const unsigned char*>(seed.data());
    auto pkey_ptr = reinterpret_cast<unsigned char*>(result.first.data());
    auto skey_ptr = reinterpret_cast<unsigned char*>(result.second.data());

    ed25519_create_keypair(pkey_ptr, skey_ptr, seed_ptr);

    return result;
}

ed25519::exch_type ed25519::generate_exch(const skey_type& a_skey, const pkey_type& b_pkey)
{
    exch_type result;

    auto skey_ptr = reinterpret_cast<const unsigned char*>(a_skey.data());
    auto pkey_ptr = reinterpret_cast<const unsigned char*>(b_pkey.data());
    auto exch_ptr = reinterpret_cast<unsigned char*>(result.data());

    ed25519_key_exchange(exch_ptr, pkey_ptr, skey_ptr);

    return result;
}

ed25519::sign_type ed25519::sign(const pair_type& pair, std::span<const std::byte> mesg)
{
    sign_type result;

    auto pkey_ptr = reinterpret_cast<const unsigned char*>(pair.first.data());
    auto skey_ptr = reinterpret_cast<const unsigned char*>(pair.second.data());
    auto mesg_ptr = reinterpret_cast<const unsigned char*>(mesg.data());
    auto sign_ptr = reinterpret_cast<unsigned char*>(result.data());

    ed25519_sign(sign_ptr, mesg_ptr, mesg.size_bytes(), pkey_ptr, skey_ptr);

    return result;
}

bool ed25519::verify(const pkey_type& pkey, std::span<const std::byte> mesg, const sign_type& sign)
{
    auto pkey_ptr = reinterpret_cast<const unsigned char*>(pkey.data());
    auto mesg_ptr = reinterpret_cast<const unsigned char*>(mesg.data());
    auto sign_ptr = reinterpret_cast<const unsigned char*>(sign.data());

    auto result = ed25519_verify(sign_ptr, mesg_ptr, mesg.size_bytes(), pkey_ptr);

    return static_cast<bool>(result);
}

std::optional<ed25519::pkey_type> ed25519::import_pkey(std::string_view xstr)
{
    return import_xstr<32>(xstr);
}

std::optional<ed25519::skey_type> ed25519::import_skey(std::string_view xstr)
{
    return import_xstr<64>(xstr);
}

std::string ed25519::export_pkey(const pkey_type& pkey)
{
    return export_xstr<32>(pkey);
}

std::string ed25519::export_skey(const skey_type& skey)
{
    return export_xstr<64>(skey);
}

std::string ed25519::export_sign(const sign_type& sign)
{
    return export_xstr<64>(sign);
}
