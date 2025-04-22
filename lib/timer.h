//
// Created by admin on 25-4-1.
//

#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <sys/epoll.h>


/**
 * 1. 触发时刻作为 key，任务作为 val
 * 2. 快速找到最近要超时的任务 O(1)
 * 3. 触发后要删除该任务且支持随时删除任务，要满足 O(1)
 * 4. 允许相同时刻触发任务
 */
class Timer;

class TimerTask
{
    friend class Timer;

public:
    using Callback = std::function<void(TimerTask *task)>;

    TimerTask(uint64_t addtime, uint64_t exectime, Callback task);

    uint64_t AddTime() const;

    uint64_t ExecTime() const;

private:
    void run();

    uint64_t m_addTime;
    uint64_t m_execTime;
    Callback m_func;
};

class Timer
{
    using Milliseconds = std::chrono::milliseconds;

public:
    static uint64_t GetTick();

    TimerTask *AddTimeOut(uint64_t offset, TimerTask::Callback func); // 添加定时器

    void DelTimeout(TimerTask *task); // 删除定时器

    void Update(uint64_t now); // 更新定时器

    int WaitTime(); // 获取最近的超时时间

private:
    std::multimap<uint64_t, TimerTask*> m_timeouts;
};

void test();
#endif //TIMER_H
