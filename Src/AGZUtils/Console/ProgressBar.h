#pragma once

#include <chrono>
#include <iostream>

namespace AGZ
{

class ProgressBar
{
    int finished_;
    int total_;

    int width_;
    char complete_;
    char incomplete_;

    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();

public:

    ProgressBar(int total, int width, char complete = '#', char incomplete = ' ')
        : finished_(0), total_(total), width_(width), complete_(complete), incomplete_(incomplete)
    {

    }

    ProgressBar &operator++()
    {
        ++finished_;
        if(finished_ >= total_)
            Done();
        return *this;
    }

    void Display() const
    {
        float progress = (float) finished_ / total_;
        int pos = (int) (width_ * progress);

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();

        std::cout << "[";

        for(int i = 0; i < width_; ++i)
        {
            if(i < pos)
                std::cout << complete_;
            else if(i == pos)
                std::cout << ">";
            else
                std::cout << incomplete_;
        }
        std::cout << "] " << int(progress * 100.0) << "% "
                  << float(time_elapsed) / 1000.0 << "s\r";
        std::cout.flush();
    }

    void Done()
    {
        Display();
        if(finished_ <= total_)
        {
            std::cout << std::endl;
            ++finished_;
        }
    }
};

class ProgressBarF
{
    float percent_;

    int width_;
    char complete_;
    char incomplete_;

    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();

public:

    ProgressBarF(int width, char complete = '#', char incomplete = ' ')
        : percent_(0), width_(width), complete_(complete), incomplete_(incomplete)
    {

    }

    void SetPercent(float percent)
    {
        percent_ = percent;
    }

    void Display() const
    {
        int pos = (int)(width_ * percent_ / 100);

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();

        std::cout << "[";

        for(int i = 0; i < width_; ++i)
        {
            if(i < pos)
                std::cout << complete_;
            else if(i == pos)
                std::cout << ">";
            else
                std::cout << incomplete_;
        }
        std::cout << "] " << int(percent_) << "% "
                  << float(time_elapsed) / 1000.0 << "s\r";
        std::cout.flush();
    }

    void Done()
    {
        Display();
        std::cout << std::endl;
    }
};

} // namespace AGZ

