#ifndef DF9E0E09_AFDE_4465_AD24_6C1147225ED4
#define DF9E0E09_AFDE_4465_AD24_6C1147225ED4

struct BotherResponseEvent final {
public:
    explicit BotherResponseEvent(unsigned request_id);
    explicit BotherResponseEvent(unsigned request_id, std::uint16_t num_players, std::uint16_t max_players, std::uint32_t version_major,
        std::uint32_t version_minor, std::uint32_t version_patch, const std::string& motd);
    constexpr unsigned request_id(void) const;
    constexpr bool unreachable(void) const;
    constexpr std::uint16_t num_players(void) const;
    constexpr std::uint16_t max_players(void) const;
    constexpr std::uint32_t version_major(void) const;
    constexpr std::uint32_t version_minor(void) const;
    constexpr std::uint32_t version_patch(void) const;
    constexpr std::string_view motd(void) const;

private:
    unsigned m_request_id;
    bool m_unreachable;
    std::uint16_t m_num_players {};
    std::uint16_t m_max_players {};
    std::uint32_t m_version_major {};
    std::uint32_t m_version_minor {};
    std::uint32_t m_version_patch {};
    std::string m_motd {};
};

namespace bother
{
void init(void);
void shutdown(void);
void update_late(void);
void ping(unsigned int request_id, std::string_view host, std::uint16_t port);
void cancel(unsigned int request_id);
} // namespace bother

constexpr unsigned BotherResponseEvent::request_id(void) const
{
    return m_request_id;
}

constexpr bool BotherResponseEvent::unreachable(void) const
{
    return m_unreachable;
}

constexpr std::uint16_t BotherResponseEvent::num_players(void) const
{
    return m_num_players;
}

constexpr std::uint16_t BotherResponseEvent::max_players(void) const
{
    return m_max_players;
}

constexpr std::uint32_t BotherResponseEvent::version_major(void) const
{
    return m_version_major;
}

constexpr std::uint32_t BotherResponseEvent::version_minor(void) const
{
    return m_version_minor;
}

constexpr std::uint32_t BotherResponseEvent::version_patch(void) const
{
    return m_version_patch;
}

constexpr std::string_view BotherResponseEvent::motd(void) const
{
    return m_motd;
}

#endif /* DF9E0E09_AFDE_4465_AD24_6C1147225ED4 */
