#include "shared/pch.hh"
#include "shared/threading.hh"

#include "core/cmdline.hh"
#include "core/constexpr.hh"

static BS::light_thread_pool *thread_pool;
static std::deque<Task *> task_deque;

static void task_process(Task *task)
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
    auto threads_arg = cmdline::get("threads", "4");
    auto threads_num = cxpr::clamp<unsigned long>(std::strtoul(threads_arg, nullptr, 10), 2U, 4U);
    thread_pool = new BS::light_thread_pool(threads_num);
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

void threading::submit(Task *task)
{
    task->set_status(task_status::ENQUEUED);

    static_cast<void>(thread_pool->submit_task(std::bind(&task_process, task)));

    task_deque.push_back(task);
}
