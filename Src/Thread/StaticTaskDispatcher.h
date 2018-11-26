#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace AGZ {

/**
 * @brief 对不包含任何共享参数的任务分配，可以此作为dummy parameter
 */
inline struct NoSharedParam_t { } NO_SHARED_PARAM;
    
/**
 * @brief 静态任务分派器
 * 
 * 将一组给定的任务分配给一定数量的线程完成
 * 
 * 在此过程中不可动态添加或删除任务，任务之间的顺序和并行性也不得到任何保证
 */
template<typename TaskType, typename SharedParamType>
class StaticTaskDispatcher
{
    int workerCount_;

    std::vector<std::exception> exceptions_;

    struct Params
    {
        const SharedParamType &sharedParam;
        std::queue<TaskType> &tasks;
        std::mutex &taskMut;

        std::vector<std::exception> &exceptions;
        std::mutex &exceptionMut;
    };

    template<typename Func>
    static void Worker(const Func &func, Params param)
    {
        try
        {
            for(;;)
            {
                TaskType task;
                {
                    std::lock_guard<std::mutex> lk(param.taskMut);
                    if(param.tasks.empty())
                        break;
                    task = param.tasks.front();
                    param.tasks.pop();
                }
                func(task, param.sharedParam);
            }
        }
        catch(const std::exception &err)
        {
            std::lock_guard<std::mutex> lk(param.exceptionMut);
            param.exceptions.push_back(err);
        }
        catch(...)
        {
            std::lock_guard<std::mutex> lk(param.exceptionMut);
            param.exceptions.push_back(
                std::runtime_error("StaticTaskDispatcher: unknown exception"));
        }
    }

public:

    /**
     * @param workerCount 完成任务的工作线程数量，为非正数时使用硬件线程数
     */
    explicit StaticTaskDispatcher(int workerCount)
    {
        if(workerCount <= 0)
            workerCount = std::thread::hardware_concurrency();
        workerCount_ = (std::max)(1, workerCount) - 1;
    }

    /**
     * @brief 完成给定的一组任务，会清空之前调用时产生的异常列表
     * 
     * @return 若完成任务的过程中没有抛出任何异常，则返回true；否则返回false。
     *         返回false时可通过 GetExceptions 查看异常列表
     */
    template<typename Func>
    bool Run(const Func &func, const SharedParamType &sharedParam, std::queue<TaskType> &tasks)
    {
        exceptions_.clear();

        std::mutex taskMut, exceptionMut;
        std::vector<std::thread> workers;

        Params params = { sharedParam, tasks, taskMut, exceptions_, exceptionMut };

        for(int i = 1; i < workerCount_; ++i)
            workers.emplace_back(&Worker<Func>, func, params);
        Worker(func, params);

        for(auto &worker : workers)
            worker.join();

        return exceptions_.empty();
    }

    /**
     * @brief 取得上一次调用 Run 时产生的异常列表
     */
    const std::vector<std::exception> &GetExceptions() const
    {
        return exceptions_;
    }
};

} // namespace AGZ
