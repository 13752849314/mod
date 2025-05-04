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

std::atomic<int> task_counter{0}; // 全局计数器，用于统计任务完成的数量
const int num_producers = 4; // 生产者线程数量
const int num_tasks_pre_producer = 10; // 每个生产者提交的任务数量
const int num_thread_in_pool = 5; // 线程池中的工作线程数量
// 任务函数
void Task(int id)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 模拟任务执行时间
    std::cout << "Task " << id << " executed by thread " << std::this_thread::get_id() << std::endl;
    ++task_counter; // 任务完成，计数器加一
}


// 生产者线程函数
void Producer(ThreadPool &pool, int producer_id, int num_tasks)
{
    for (int i = 0; i < num_tasks; ++i)
    {
        int task_id = producer_id * 1000 + i; // 生成任务唯一id
        pool.Post(Task, task_id); // 提交任务到线程池
        // pool.Post(lab, task_id);
        std::cout << "Prouccer " << producer_id << " post task" << task_id << std::endl;
    }
}

int main()
{
    // ThreadPool pool(4);
    // auto task1 = pool.Post(add, 1, 2);
    // auto task2 = pool.Post([](int a, int b)
    // {
    //     std::this_thread::sleep_for(std::chrono::seconds(5));
    //     return a * b;
    // }, 1, 2);
    //
    // auto task3 = pool.Post([](int a, int b)
    // {
    //     std::this_thread::sleep_for(std::chrono::seconds(5));
    //     return std::to_string(a) + std::to_string(b);
    // }, 1, 2);
    //
    // std::cout << task1.get() << std::endl;
    // std::cout << task2.get() << std::endl;
    // std::cout << task3.get() << std::endl;

    ThreadPool pool(num_thread_in_pool); // 创建线程池
    std::vector<std::thread> producers; // 生产者线程集合
    // 启动生产者线程
    producers.reserve(num_producers);
    for (int i = 0; i < num_producers; ++i)
    {
        producers.emplace_back(Producer, std::ref(pool), i, num_tasks_pre_producer);
    }

    // 等待所有生产者线程完成
    for (auto &producer : producers)
    {
        producer.join();
    }

    // 等待所有任务完成
    while (task_counter < num_producers * num_tasks_pre_producer)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "All tasks completed. Total tasks executed: " << task_counter << std::endl;
    return 0;
}
