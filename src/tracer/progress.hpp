#pragma once

#include <chrono>
#include <mutex>

class ProgressBar {
public:
    void Start(const int maxValue) {
        m_Value = 0;
        m_MaxValue = maxValue;
        m_StartTime = std::chrono::steady_clock::now();
    }

    void Increment(const int delta = 1) {
        std::lock_guard<std::mutex> guard(m_Mutex);
        m_Value += delta;
        Display();
    }

    void Done() {
        Display();
        printf("\n");
    }

private:
    void Display() {
        const int pct = m_Value * 100 / m_MaxValue;
        const int ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_StartTime).count();
        printf("  %4d / %d (%3d%%) [", m_Value, m_MaxValue, pct);
        for (int p = 0; p < 100; p += 3) {
            if (pct > p) {
                printf("=");
            } else {
                printf(" ");
            }
        }
        printf("] %.3fs    \r", ms / 1000.0);
        fflush(stdout);
    }

    std::mutex m_Mutex;
    int m_Value;
    int m_MaxValue;
    std::chrono::steady_clock::time_point m_StartTime;
};
