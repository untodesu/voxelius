#ifndef DA126012_C7C9_4206_99A3_0C1A16BFC2AF
#define DA126012_C7C9_4206_99A3_0C1A16BFC2AF

enum session_state {
    SESSION_DISCONNECTED = 0,
    SESSION_CONNECTING,
    SESSION_AUTHENTICATING,
    SESSION_SPAWNING,
    SESSION_INGAME,
};

class SessionStateEvent final {
public:
    constexpr SessionStateEvent(session_state state, std::uint32_t reason = UINT32_MAX);
    constexpr session_state state(void) const;
    constexpr std::uint32_t reason(void) const;

private:
    session_state m_state;
    std::uint32_t m_reason;
};

namespace session
{
extern session_state state;
extern std::uint64_t identity;
extern std::uint16_t client_id;
extern std::string assigned_username;
} // namespace session

namespace session
{
void init(void);
void shutdown(void);
} // namespace session

namespace session
{
void connect(std::string_view host, std::uint16_t port, std::uint64_t invite);
void disconnect(std::uint32_t reason);
} // namespace session

namespace session
{
void notify_spawned(void);
} // namespace session

constexpr SessionStateEvent::SessionStateEvent(session_state state, std::uint32_t reason) : m_state(state), m_reason(reason)
{
    // empty
}

constexpr session_state SessionStateEvent::state(void) const
{
    return m_state;
}

constexpr std::uint32_t SessionStateEvent::reason(void) const
{
    return m_reason;
}

#endif /* DA126012_C7C9_4206_99A3_0C1A16BFC2AF */
