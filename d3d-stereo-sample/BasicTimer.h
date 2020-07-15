#pragma once

// A simple timer class used to drive the render loop of the actively-animated
// DirectX SDK samples. Uses the QueryPerformanceCounter function to retrieve
// high-resolution timing information.
class BasicTimer
{
private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_currentTime;
    LARGE_INTEGER m_startTime;
    LARGE_INTEGER m_lastTime;
    float m_total;
    float m_delta;

public:
    BasicTimer();
    void Reset();
    void Update();

    float Total();
    float Delta();
};