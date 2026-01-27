#pragma once
#include <chrono>
#include <iostream>
#include <string>

class ScopeTimer
{
public:
    // 构造函数可传入名字，方便区分不同计时器
    explicit ScopeTimer(const std::string& name = "ScopeTimer")
        : m_name(name), m_start(std::chrono::high_resolution_clock::now())
    {
    }

    // 析构时打印耗时
    ~ScopeTimer()
    {
        using namespace std::chrono;
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - m_start).count();
        std::cout << "[" << m_name << "] elapsed: " << duration << " ms\n";
    }

    // 可以手动获取毫秒
    long long elapsed_ms() const
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        return duration_cast<milliseconds>(now - m_start).count();
    }

private:
    std::string m_name;
    std::chrono::high_resolution_clock::time_point m_start;
};
