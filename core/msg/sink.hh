#ifndef E3C97FAC_A68A_4EB8_AA4A_769DF90BB2A2
#define E3C97FAC_A68A_4EB8_AA4A_769DF90BB2A2

namespace msg
{
using level_type = std::uint8_t;
constexpr static level_type LEVEL_DEBUG = 0;
constexpr static level_type LEVEL_INFO = 1;
constexpr static level_type LEVEL_WARNING = 2;
constexpr static level_type LEVEL_ERROR = 3;
constexpr static level_type LEVEL_CRITICAL = 4;
} // namespace msg

namespace msg
{
struct Record final {
    level_type level;
    std::source_location where;
    std::string_view message;
};
} // namespace msg

namespace msg
{
using sink_type = entt::delegate<void(const Record& record)>;
} // namespace msg

namespace msg::detail
{
void stderr_sink(const Record& record);
} // namespace msg::detail

namespace msg::detail
{
void add_sink(sink_type sink);
void remove_sink(sink_type sink);
} // namespace msg::detail

namespace msg
{
template<auto Candidate>
void add_sink(void);
template<auto Candidate>
void remove_sink(void);
} // namespace msg

namespace msg
{
void submit(const Record& record);
} // namespace msg

template<auto Candidate>
void msg::add_sink(void)
{
    sink_type sink {};
    sink.connect<Candidate>();

    detail::add_sink(std::move(sink));
}

template<auto Candidate>
void msg::remove_sink(void)
{
    sink_type sink {};
    sink.connect<Candidate>();

    detail::remove_sink(std::move(sink));
}

#endif /* E3C97FAC_A68A_4EB8_AA4A_769DF90BB2A2 */
