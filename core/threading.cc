#include "core/pch.hh"

#include "core/threading.hh"

#include "core/io/cmdline.hh"

#include "core/math/constexpr.hh"

constexpr static std::string_view DEFAULT_POOL_SIZE_ARG = "4";

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
    auto argument = io::cmdline::get("threads", DEFAULT_POOL_SIZE_ARG);
    auto num_concurrent_threads = std::thread::hardware_concurrency();
    unsigned int thread_pool_size;

    if(num_concurrent_threads && 0 == argument.compare("max")) {
        // Use the maximum available number of concurrent
        // hardware threads provided by the implementation
        thread_pool_size = num_concurrent_threads;
    }
    else {
        if(num_concurrent_threads) {
            auto result = std::from_chars(argument.data(), argument.data() + argument.size(), thread_pool_size);

            if(result.ec == std::errc()) {
                thread_pool_size = glm::clamp<unsigned int>(thread_pool_size, 1U, num_concurrent_threads);
            }
            else {
                thread_pool_size = 4U;
            }
        }
        else {
            auto result = std::from_chars(argument.data(), argument.data() + argument.size(), thread_pool_size);

            if(result.ec == std::errc()) {
                thread_pool_size = glm::max<unsigned int>(thread_pool_size, 1U);
            }
            else {
                thread_pool_size = 4U;
            }
        }
    }

    spdlog::info("threading: using {} threads for pooling tasks", thread_pool_size);

    thread_pool = new BS::light_thread_pool(thread_pool_size);

    task_deque.clear();
}

void threading::shutdown(void)
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
