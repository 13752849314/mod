//
// Created by admin on 25-4-22.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
class BlockingQueuePro
{
public:
    explicit BlockingQueuePro(bool nonblock = false);

    void Push(const T &value); // 入队操作

    bool Pop(T &value); // 出队操作

    void Cancel(); // 解除阻塞在当前队列的线程
private:
    // 当消费者队列为空时，交换生产者和消费者队列
    int SwapQueue_();

    bool nonblock_;
    std::queue<T> producer_queue_;
    std::queue<T> consumer_queue_;
    std::mutex producer_mutex_;
    std::mutex consumer_mutex_;
    std::condition_variable not_empty_;
};

class ThreadPool
{
public:
    explicit ThreadPool(int num_threads);

    ~ThreadPool();

    /**
     * 发布任务到线程池
     * @tparam F 可调用对象（函数，lambda表达式，函数对象等）
     * @tparam Args 可变参数模板，表示可以接受任意数量的参数
     * @param f 任务对象
     * @param args 任务参数
     */
    template <typename F, typename... Args>
    void Post(F &&f, Args &&... args);

private:
    void Worker();
    BlockingQueuePro<std::function<void()>> task_queue_; // 任务队列
    std::vector<std::thread> workers_; // 工作线程
};

#endif //THREADPOOL_H
