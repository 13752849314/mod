//
// Created by admin on 25-3-30.
//

#include "shared_ptr.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

template <typename T>
void shared_ptr<T>::release() const
{
    // std::memory_order_acq_rel 保证释放资源的同步
    if (ref_count && ref_count->fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        ptr = nullptr;
        delete ptr;
        delete ref_count;
    }
}

template <typename T>
shared_ptr<T>::shared_ptr() :
    ptr(nullptr), ref_count(nullptr)
{
}

template <typename T>
shared_ptr<T>::shared_ptr(T *p):
    ptr(p), ref_count(p ? new std::atomic<std::size_t>(1) : nullptr)
{
}

template <typename T>
shared_ptr<T>::~shared_ptr()
{
    release();
}

template <typename T>
shared_ptr<T>::shared_ptr(const shared_ptr<T> &other):
    ptr(other.ptr), ref_count(other.ref_count)
{
    if (ref_count)
    {
        ref_count->fetch_add(1, std::memory_order_relaxed); // 引用计数加1
    }
}

template <typename T>
shared_ptr<T> &shared_ptr<T>::operator=(const shared_ptr<T> &other)
{
    if (this != &other)
    {
        release(); // 释放当前资源
        ptr = other.ptr;
        ref_count = other.ref_count;
        if (ref_count)
        {
            ref_count->fetch_add(1, std::memory_order_relaxed); // 引用计数加1
        }
    }
    return *this;
}

template <typename T>
shared_ptr<T>::shared_ptr(shared_ptr<T> &&other) noexcept :
    ptr(other.ptr), ref_count(other.ref_count)
{
    other.ptr = nullptr;
    other.ref_count = nullptr;
}

template <typename T>
shared_ptr<T> &shared_ptr<T>::operator=(shared_ptr<T> &&other)
    noexcept
{
    if (this != &other)
    {
        release();
        ptr = other.ptr;
        ref_count = other.ref_count;
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }
    return *this;
}

template <typename T>
T &shared_ptr<T>::operator*() const
{
    return *ptr;
}

template <typename T>
T *shared_ptr<T>::operator->() const
{
    return ptr;
}

template <typename T>
std::size_t shared_ptr<T>::use_count() const
{
    return ref_count ? ref_count->load(std::memory_order_acquire) : 0;
}

template <typename T>
T *shared_ptr<T>::get() const
{
    return ptr;
}

template <typename T>
void shared_ptr<T>::reset(T *p)
{
    release();
    ptr = p;
    ref_count = p ? new std::atomic<std::size_t>(1) : nullptr;
}

std::mutex mutex;

void test()
{
    shared_ptr<int> ptr(new int(11));
    constexpr int num_threads = 200;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ptr]()
        {
            std::cout << "thread: " << std::this_thread::get_id() << std::endl;
            for (int j = 0; j < 1000; ++j)
            {
                shared_ptr<int> local_ptr(ptr);
                mutex.lock();
                (*local_ptr)++;
                mutex.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
    std::cout << ptr.use_count() << std::endl;
    std::cout << *ptr << std::endl;
}
