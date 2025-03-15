#ifndef SHARED_THREADING_HH
#define SHARED_THREADING_HH 1
#pragma once

enum class task_status : unsigned int {
    ENQUEUED    = 0x0000U,
    PROCESSING  = 0x0001U,
    COMPLETED   = 0x0002U,
    CANCELLED   = 0x0004U,
};

class Task {
public:
    virtual ~Task(void) = default;
    virtual void process(void) = 0;
    virtual void finalize(void) = 0;

    task_status get_status(void) const;
    void set_status(task_status status);

protected:
    std::atomic<task_status> m_status;
    std::future<void> m_future;
};

namespace threading
{
void init(void);
void deinit(void);
void update(void);
void submit(Task *task);
} // namespace threading

namespace threading
{
template<typename T, typename... AT>
void submit(AT &&... args);
} // namespace threading

template<typename T, typename... AT>
inline void threading::submit(AT &&... args)
{
    threading::submit(new T(args...));
}

#endif /* SHARED_THREADING_HH  */
