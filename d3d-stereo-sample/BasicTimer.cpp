#include "pch.h"
#include "BasicTimer.h"

BasicTimer::BasicTimer()
{
    if (!QueryPerformanceFrequency(&m_frequency))
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }
    Reset();
}

void BasicTimer::Reset()
{
    Update();
    m_startTime = m_currentTime;
    m_total = 0.0f;
    m_delta = 1.0f / 60.0f;
}

void BasicTimer::Update()
{
    if (!QueryPerformanceCounter(&m_currentTime))
    {
        throw winrt::hresult_error(E_UNEXPECTED);
    }

    m_total = static_cast<float>(
        static_cast<double>(m_currentTime.QuadPart - m_startTime.QuadPart) /
        static_cast<double>(m_frequency.QuadPart)
        );

    if (m_lastTime.QuadPart == m_startTime.QuadPart)
    {
        // If the timer was just reset, report a time delta equivalent to 60Hz frame time.
        m_delta = 1.0f / 60.0f;
    }
    else
    {
        m_delta = static_cast<float>(
            static_cast<double>(m_currentTime.QuadPart - m_lastTime.QuadPart) /
            static_cast<double>(m_frequency.QuadPart)
            );
    }

    m_lastTime = m_currentTime;
}

float BasicTimer::Total()
{
    return m_total;
}

float BasicTimer::Delta()
{
    return m_delta;
}