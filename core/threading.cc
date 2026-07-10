#include "core/pch.hh"

#include "core/threading.hh"

#include "core/cmdline.hh"

constexpr static std::string_view DEFAULT_POOL_SIZE_ARG = "4";

static std::unique_ptr<BS::light_thread_pool> s_threads;
static std::deque<std::unique_ptr<Task>> s_deque;

static void task_process(Task* task)
{
    task->status.store(task_status::PROCESSING, std::memory_order_relaxed);
    task->process();

    if(task_status::PROCESSING == task->status.load(std::memory_order_relaxed)) {
        task->status.store(task_status::COMPLETED, std::memory_order_relaxed);
    }
}

void threading::detail::submit_new(std::unique_ptr<Task>&& task)
{
    task->status.store(task_status::ENQUEUED, std::memory_order_relaxed);
    task->future = s_threads->submit_task(std::bind(&task_process, task.get()));
    s_deque.push_back(std::move(task));
}

void threading::init(void)
{
    auto argument = cmdline::value("threads").value_or(DEFAULT_POOL_SIZE_ARG);
    auto num_concurrent_threads = std::thread::hardware_concurrency();
    unsigned thread_pool_size;

    if(num_concurrent_threads && 0 == argument.compare("max")) {
        // Use the maximum available number of concurrent
        // hardware threads provided by the implementation
        thread_pool_size = num_concurrent_threads;
    }
    else if(num_concurrent_threads) {
        auto result = std::from_chars(argument.data(), argument.data() + argument.size(), thread_pool_size);

        if(result.ec == std::errc()) {
            thread_pool_size = std::clamp<unsigned>(thread_pool_size, 1U, num_concurrent_threads);
        }
        else {
            thread_pool_size = 4U;
        }
    }
    else {
        auto result = std::from_chars(argument.data(), argument.data() + argument.size(), thread_pool_size);

        if(result.ec == std::errc()) {
            thread_pool_size = std::max<unsigned>(thread_pool_size, 1U);
        }
        else {
            thread_pool_size = 4U;
        }
    }

    LOG_INFO("using {} threads for pooling tasks", thread_pool_size);

    s_threads = std::make_unique<BS::light_thread_pool>(thread_pool_size);
    s_deque.clear();
}

void threading::shutdown(void)
{
    for(auto& it : s_deque) {
        auto status = it->status.load(std::memory_order_relaxed);

        if(status == task_status::ENQUEUED || status == task_status::PROCESSING) {
            it->status.store(task_status::CANCELLED, std::memory_order_relaxed);
        }
    }

    s_threads->purge();
    s_threads->wait();

    s_deque.clear();
    s_threads.reset();
}

void threading::update(void)
{
    auto it = s_deque.cbegin();

    while(it != s_deque.cend()) {
        auto task_ptr = it->get();
        auto status = task_ptr->status.load(std::memory_order_relaxed);

        if(status == task_status::CANCELLED) {
            it = s_deque.erase(it);
            continue;
        }

        if(status == task_status::COMPLETED) {
            task_ptr->finalize();
            it = s_deque.erase(it);
            continue;
        }

        it = std::next(it);
    }
}
