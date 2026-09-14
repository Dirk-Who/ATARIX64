/* AtariX - GPL-3.0-or-later. Host display scheduling, independent of guest timers. */
#ifndef ATARIX_DISPLAY_UPDATE_SCHEDULER_H
#define ATARIX_DISPLAY_UPDATE_SCHEDULER_H

#include <atomic>

class DisplayUpdateScheduler
{
public:
    DisplayUpdateScheduler() : m_refreshRate(50), m_pending(false) {}

    void SetRefreshRate(unsigned hz)
    {
        m_refreshRate.store(hz == 25 ? 25 : 50, std::memory_order_relaxed);
    }

    unsigned GetRefreshRate() const
    {
        return m_refreshRate.load(std::memory_order_relaxed);
    }

    // Called by the 200 Hz SDL timer. Hold one slot until the main thread
    // finishes rendering; a failed/filtered event push must release it too.
    bool TrySchedule(unsigned tick, bool dirty)
    {
        if (!dirty || tick % (200 / GetRefreshRate()) != 0)
            return false;
        bool expected = false;
        return m_pending.compare_exchange_strong(expected, true,
                    std::memory_order_acq_rel, std::memory_order_relaxed);
    }

    void FinishUpdate()
    {
        m_pending.store(false, std::memory_order_release);
    }

private:
    std::atomic<unsigned> m_refreshRate;
    std::atomic<bool> m_pending;
};

#endif
