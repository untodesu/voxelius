#include "core/pch.hh"

#include "core/msg/capture.hh"

msg::ScopedCapture::ScopedCapture(level_type min_level, std::size_t max_records)
    : m_thread_id(std::this_thread::get_id()), m_min_level(min_level), m_max_records(max_records)
{
    m_records.reserve(max_records);

    sink_type sink {};
    sink.connect<&ScopedCapture::sink_callback>(this);

    detail::add_sink(sink);
}

msg::ScopedCapture::~ScopedCapture(void)
{
    sink_type sink {};
    sink.connect<&ScopedCapture::sink_callback>(this);

    detail::remove_sink(sink);
}

void msg::ScopedCapture::sink_callback(const Record& record)
{
    if(m_thread_id == std::this_thread::get_id()) {
        if(record.level < m_min_level) {
            return;
        }

        if(m_records.size() >= m_max_records) {
            return;
        }

        CaptureEntry entry {};
        entry.level = record.level;
        entry.where = record.where;
        entry.message = record.message;

        m_records.emplace_back(std::move(entry));
    }
}
