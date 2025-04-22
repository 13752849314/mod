//
// Created by admin on 25-4-22.
//

#include "ThreadPool.h"

template <typename T>
BlockingQueuePro<T>::BlockingQueuePro(bool nonblock):
    nonblock_(nonblock)
{
}

template <typename T>
void BlockingQueuePro<T>::Push(const T &value)
{
    std::lock_guard<std::mutex> lock(producer_mutex_);
    producer_queue_.push(value);
    not_empty_.notify_one();
}

template <typename T>
bool BlockingQueuePro<T>::Pop(T &value)
{
    std::unique_lock<std::mutex> lock(consumer_mutex_);
    if (consumer_queue_.empty() && SwapQueue_() == 0)
    {
        return false;
    }
    value = consumer_queue_.front();
    consumer_queue_.pop();
    return true;
}

template <typename T>
void BlockingQueuePro<T>::Cancel()
{
    std::lock_guard<std::mutex> lock(producer_mutex_);
    nonblock_ = true;
    not_empty_.notify_all();
}

template <typename T>
int BlockingQueuePro<T>::SwapQueue_()
{
    std::unique_lock<std::mutex> lock(producer_mutex_);
    not_empty_.wait(lock, [this]
    {
        return !producer_queue_.empty() || nonblock_;
    });
    std::swap(producer_queue_, consumer_queue_);
    return consumer_queue_.size();
}

template <typename F, typename... Args>
void ThreadPool::Post(F &&f, Args &&... args)
{
    auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    task_queue_.Push(task);
}

ThreadPool::ThreadPool(int num_threads)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this]
        {
            Worker();
        });
    }
}

ThreadPool::~ThreadPool()
{
    task_queue_.Cancel();
    for (auto &worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::Worker()
{
    while (true)
    {
        std::function<void()> task;
        if (!task_queue_.Pop(task))
        {
            break;
        }
        task();
    }
}
