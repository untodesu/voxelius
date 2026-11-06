#include "core/pch.hh"

#include "core/msg/sink.hh"

static std::mutex s_mutex;
static std::vector<msg::sink_type> s_sinks;

static const char* level_name(msg::level_type level)
{
    switch(level) {
        case msg::LEVEL_INFO:
            return "info";

        case msg::LEVEL_WARNING:
            return "warning";

        case msg::LEVEL_ERROR:
            return "error";

        case msg::LEVEL_CRITICAL:
            return "critical";

        case msg::LEVEL_DEBUG:
            return "debug";
    }

    return "unknown";
}

static const char* level_ansi(msg::level_type level)
{
    switch(level) {
        case msg::LEVEL_INFO:
            return "1;32";

        case msg::LEVEL_WARNING:
            return "1;33";

        case msg::LEVEL_ERROR:
            return "1;31";

        case msg::LEVEL_CRITICAL:
            return "1;41";

        case msg::LEVEL_DEBUG:
            return "1;34";
    }

    return "1;35";
}

static void builtin_sink(std::FILE* fd, const msg::Record& record)
{
    assert(fd);

    auto path_file = std::filesystem::path(record.where.file_name()).filename().string();
    auto message_size = static_cast<int>(record.message.size());

    std::fprintf(fd, "\033[%sm[%s]\033[0m %s:%lu: %.*s\r\n", level_ansi(record.level), level_name(record.level), path_file.c_str(),
        static_cast<unsigned long>(record.where.line()), message_size, record.message.data());
}

void msg::detail::stderr_sink(const Record& record)
{
    builtin_sink(stderr, record);
}

void msg::detail::add_sink(sink_type sink)
{
    assert(static_cast<bool>(sink));

    std::lock_guard locker(s_mutex);

    auto it = std::ranges::find(s_sinks, sink);
    assert(it == s_sinks.end());

    s_sinks.emplace_back(std::move(sink));
}

void msg::detail::remove_sink(sink_type sink)
{
    assert(static_cast<bool>(sink));

    std::lock_guard locker(s_mutex);

    std::erase(s_sinks, sink);
}

void msg::submit(const Record& record)
{
    std::lock_guard locker(s_mutex);

    for(auto& sink : s_sinks) {
        sink(record);
    }
}
