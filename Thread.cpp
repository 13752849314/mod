//
// Created by admin on 25-5-4.
//
#include <future>
#include <iostream>
#include <memory>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
class BlockingQueuePro
{
public:
    explicit BlockingQueuePro(bool nonblock = false)
    {
        nonblock_ = nonblock;
    }

    void Push(const T &value)
    {
        std::lock_guard<std::mutex> lock(producer_mutex_);
        producer_queue_.push(value);
        not_empty_.notify_one();
    } // 入队操作

    bool Pop(T &value)
    {
        std::unique_lock<std::mutex> lock(consumer_mutex_);
        if (consumer_queue_.empty() && SwapQueue_() == 0)
        {
            return false;
        }
        value = consumer_queue_.front();
        consumer_queue_.pop();
        return true;
    } // 出队操作

    void Cancel()
    {
        std::lock_guard<std::mutex> lock(producer_mutex_);
        nonblock_ = true;
        not_empty_.notify_all();
    } // 解除阻塞在当前队列的线程
private:
    // 当消费者队列为空时，交换生产者和消费者队列
    int SwapQueue_()
    {
        std::unique_lock<std::mutex> lock(producer_mutex_);
        not_empty_.wait(lock, [this]
        {
            return !producer_queue_.empty() || nonblock_;
        });
        std::swap(producer_queue_, consumer_queue_);
        return consumer_queue_.size();
    }

    bool nonblock_;
    std::queue<T> producer_queue_;
    std::queue<T> consumer_queue_;
    std::mutex producer_mutex_;
    std::mutex consumer_mutex_;
    std::condition_variable not_empty_;
};

template <typename T>
class Queue
{
public:

private:
    // 线程池中的线程
    std::vector<std::thread> workers;
    // 任务队列
    std::queue<std::function<void()>> tasks;

    // 互斥锁，保护任务队列
    std::mutex queue_mutex;
    // 条件变量，用于线程间同步
    std::condition_variable condition;
    // 线程池停止标志
    bool stop;
};

class ThreadPool
{
public:
    explicit ThreadPool(int num_threads)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers_.emplace_back([this]
            {
                Worker();
            });
        }
    }

    ~ThreadPool()
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

    /**
     * 发布任务到线程池
     * @tparam F 可调用对象（函数，lambda表达式，函数对象等）
     * @tparam Args 可变参数模板，表示可以接受任意数量的参数
     * @param f 任务对象
     * @param args 任务参数
     */
    template <typename F, typename... Args>
    auto Post(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using RetType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        const std::function<void()> func = [task]()
        {
            (*task)();
        };
        task_queue_.Push(func);
        return task->get_future();
    }

private:
    void Worker()
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

    BlockingQueuePro<std::function<void()>> task_queue_; // 任务队列
    std::vector<std::thread> workers_; // 工作线程
};

int add(int a, int b)
{
    return a + b;
}

int main()
{
    ThreadPool pool(4);
    auto task1 = pool.Post(add, 1, 2);
    auto task2 = pool.Post([](int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return a * b;
    }, 1, 2);

    auto task3 = pool.Post([](int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return std::to_string(a) + std::to_string(b);
    }, 1, 2);

    std::cout << task1.get() << std::endl;
    std::cout << task2.get() << std::endl;
    std::cout << task3.get() << std::endl;
    return 0;
}
