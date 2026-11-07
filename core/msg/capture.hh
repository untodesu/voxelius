#ifndef B941C1B4_D4A5_48BA_B0ED_9D004FD7EB78
#define B941C1B4_D4A5_48BA_B0ED_9D004FD7EB78

#include "core/msg/sink.hh"

namespace msg
{
struct CaptureEntry final {
    level_type level;
    std::source_location where;
    std::string message;
};
} // namespace msg

namespace msg
{
class ScopedCapture final {
public:
    explicit ScopedCapture(level_type min_level = LEVEL_INFO, std::size_t max_records = 32);
    ~ScopedCapture(void);

    ScopedCapture(const ScopedCapture& other) = delete;
    ScopedCapture& operator=(const ScopedCapture& other) = delete;

    ScopedCapture(ScopedCapture&& other) = delete;
    ScopedCapture& operator=(ScopedCapture&& other) = delete;

    constexpr bool empty(void) const;
    constexpr bool has_records(void) const;
    constexpr std::span<const CaptureEntry> records(void) const;

private:
    void sink_callback(const Record& record);

    std::thread::id m_thread_id;
    level_type m_min_level;
    std::size_t m_max_records;
    std::vector<CaptureEntry> m_records;
};
} // namespace msg

constexpr bool msg::ScopedCapture::empty(void) const
{
    return m_records.empty();
}

constexpr bool msg::ScopedCapture::has_records(void) const
{
    return static_cast<bool>(m_records.size());
}

constexpr std::span<const msg::CaptureEntry> msg::ScopedCapture::records(void) const
{
    return m_records;
}

#endif /* B941C1B4_D4A5_48BA_B0ED_9D004FD7EB78 */
