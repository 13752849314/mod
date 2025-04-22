//
// Created by admin on 25-4-1.
//

#include "timer.h"

#include <iostream>
#include <memory>

TimerTask::TimerTask(uint64_t addtime, uint64_t exectime, Callback task)
{
    m_addTime = addtime;
    m_execTime = exectime;
    m_func = std::move(task);
}

uint64_t TimerTask::AddTime() const
{
    return m_addTime;
}

uint64_t TimerTask::ExecTime() const
{
    return m_execTime;
}

void TimerTask::run()
{
    m_func(this);
}

uint64_t Timer::GetTick()
{
    auto sc = std::chrono::time_point_cast<Milliseconds>(std::chrono::steady_clock::now());
    auto temp = std::chrono::duration_cast<Milliseconds>(sc.time_since_epoch());
    return temp.count();
}

TimerTask *Timer::AddTimeOut(uint64_t offset, TimerTask::Callback func)
{
    auto now = GetTick();
    auto exectime = now + offset;
    auto task = new TimerTask(now, exectime, std::move(func));
    if (!m_timeouts.empty() && exectime >= m_timeouts.crbegin()->first)
    {
        auto ele = m_timeouts.emplace_hint(m_timeouts.end(), exectime, task);
        return ele->second;
    }
    auto ele = m_timeouts.emplace(exectime, task);
    return ele->second;
}

void Timer::DelTimeout(TimerTask *task)
{
    auto range = m_timeouts.equal_range(task->ExecTime());
    for (auto itr = range.first; itr != range.second;)
    {
        if (itr->second == task)
        {
            itr = m_timeouts.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}

void Timer::Update(uint64_t now)
{
    auto itr = m_timeouts.begin();
    while (itr != m_timeouts.end() && itr->first <= now)
    {
        itr->second->run();
        delete itr->second;
        itr = m_timeouts.erase(itr);
    }
}

int Timer::WaitTime()
{
    auto itr = m_timeouts.begin();
    if (itr == m_timeouts.end())
    {
        return -1;
    }
    int diss = static_cast<int>(itr->first - GetTick());
    return diss > 0 ? diss : 0;
}


void test()
{
    int epfd = epoll_create(1);
    // std::unique_ptr<Timer> timer = std::unique_ptr<Timer>();
    auto *timer = new Timer();
    int i = 0;

    timer->AddTimeOut(1000, [&](TimerTask *task)
    {
        std::cout << Timer::GetTick() << " addTime:" << task->AddTime() << " revoked times:" << ++i << std::endl;
    });

    timer->AddTimeOut(2000, [&](TimerTask *task)
    {
        std::cout << Timer::GetTick() << " addTime:" << task->AddTime() << " revoked times:" << ++i << std::endl;
    });

    timer->AddTimeOut(3000, [&](TimerTask *task)
    {
        std::cout << Timer::GetTick() << " addTime:" << task->AddTime() << " revoked times:" << ++i << std::endl;
    });

    auto task1 = timer->AddTimeOut(2100, [&](TimerTask *task)
    {
        std::cout << Timer::GetTick() << " addTime:" << task->AddTime() << " revoked times:" << ++i << std::endl;
    });

    timer->DelTimeout(task1);

    std::cout << "now time:" << Timer::GetTick() << std::endl;

    epoll_event ev[64] = {0};

    while (true)
    {
        std::cout << "Wait time:" << timer->WaitTime() << std::endl;
        int n = epoll_wait(epfd, ev, 64, timer->WaitTime());
        uint64_t now = Timer::GetTick();
        for (int j = 0; j < n; j++)
        {

        }
        timer->Update(now);
    }
}
