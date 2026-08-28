/*
 * Copyright (C) 2026
 *
 * Small C++ synchronization primitives used instead of the Carbon
 * Multiprocessing Services API, which is unavailable on Apple Silicon.
 */

#ifndef ATARIX_MODERN_SYNC_H
#define ATARIX_MODERN_SYNC_H

#include <condition_variable>
#include <cstdint>
#include <mutex>

class AtariEvent
{
public:
	void Set(uint32_t flags)
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Flags |= flags;
		}
		m_Condition.notify_one();
	}

	uint32_t Wait()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Condition.wait(lock, [this] { return m_Flags != 0; });

		const uint32_t flags = m_Flags;
		m_Flags = 0;
		return flags;
	}

	void Reset()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Flags = 0;
	}

private:
	std::mutex m_Mutex;
	std::condition_variable m_Condition;
	uint32_t m_Flags = 0;
};

using AtariRecursiveMutex = std::recursive_mutex;

#endif
