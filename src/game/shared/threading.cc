#include "shared/pch.hh"

#include "shared/threading.hh"

#include "core/cmdline.hh"
#include "core/constexpr.hh"

constexpr static const char* DEFAULT_POOL_SIZE_ARG = "4";

static BS::light_thread_pool* thread_pool;
static std::deque<Task*> task_deque;

static void task_process(Task* task)
{
    task->set_status(task_status::PROCESSING);
    task->process();

    if(task->get_status() == task_status::PROCESSING) {
        // If the task status is still PROCESSING
        // it can be deduced it hasn't been cancelled
        task->set_status(task_status::COMPLETED);
    }
}

task_status Task::get_status(void) const
{
    return m_status;
}

void Task::set_status(task_status status)
{
    m_status = status;
}

void threading::init(void)
{
    auto argument = cmdline::get("threads", DEFAULT_POOL_SIZE_ARG);
    auto num_concurrent_threads = std::thread::hardware_concurrency();
    unsigned int thread_pool_size;

    if(num_concurrent_threads && !std::strcmp(argument, "max")) {
        // Use the maximum available number of concurrent
        // hardware threads provided by the implementation
        thread_pool_size = num_concurrent_threads;
    } else {
        if(num_concurrent_threads) {
            thread_pool_size = vx::clamp<unsigned int>(std::strtoul(argument, nullptr, 10), 1U, num_concurrent_threads);
        } else {
            thread_pool_size = vx::max<unsigned int>(std::strtoul(argument, nullptr, 10), 1U);
        }
    }

    spdlog::info("threading: using {} threads for pooling tasks", thread_pool_size);

    thread_pool = new BS::light_thread_pool(thread_pool_size);

    task_deque.clear();
}

void threading::deinit(void)
{
    for(auto task : task_deque) {
        auto status = task->get_status();
        if((status != task_status::CANCELLED) || (status != task_status::COMPLETED)) {
            task->set_status(task_status::CANCELLED);
        }
    }

    thread_pool->purge();
    thread_pool->wait();

    for(auto task : task_deque)
        delete task;
    task_deque.clear();

    delete thread_pool;
}

void threading::update(void)
{
    auto task_iter = task_deque.cbegin();

    while(task_iter != task_deque.cend()) {
        auto task_ptr = *task_iter;
        auto status = task_ptr->get_status();

        if(status == task_status::CANCELLED) {
            delete task_ptr;
            task_iter = task_deque.erase(task_iter);
            continue;
        }

        if(status == task_status::COMPLETED) {
            task_ptr->finalize();
            delete task_ptr;
            task_iter = task_deque.erase(task_iter);
            continue;
        }

        task_iter = std::next(task_iter);
    }
}

void threading::detail::submit_new(Task* task)
{
    task->set_status(task_status::ENQUEUED);

    static_cast<void>(thread_pool->submit_task(std::bind(&task_process, task)));

    task_deque.push_back(task);
}
