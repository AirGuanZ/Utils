#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <AGZUtils/Misc/Common.h>

namespace AGZ {

/**
 * @brief 对不包含任何共享参数的任务分配，可以此作为dummy parameter
 */
inline struct NoSharedParam_t { } NO_SHARED_PARAM;
    
/**
 * @brief 静态任务分派器
 * 
 * 将一组给定的任务分配给一定数量的线程完成。在此过程中不可动态添加或删除任务，任务之间的顺序和并行性也不得到任何保证。
 * 
 * StaticTaskDispatcher大致有两种使用方式。一是调用Run，将一组任务分配到一定数量的工作线程中，调用方将被阻塞，直到所有任务都被完成为止。
 * 二是调用RunAsync以异步地将任务分派出去，之后可以通过Join来等待所有任务完成，或通过IsCompleted来查询是否已经完成所有任务。
 */
template<typename TaskType, typename SharedParamType = NoSharedParam_t>
class StaticTaskDispatcher
{
    int workerCount_;
    mutable std::mutex taskMut_;
    std::mutex exceptionMut_;

    std::queue<TaskType> tasks_;
    std::vector<std::thread> workers_;
    std::vector<std::exception> exceptions_;

    struct Params
    {
        const SharedParamType &sharedParam;
        std::queue<TaskType> &tasks;
        std::mutex &taskMut;

        std::vector<std::exception> &exceptions;
        std::mutex &exceptionMut;
    };

    std::unique_ptr<Params> params_;

    template<typename Func>
    static void Worker(const Func &func, Params param)
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

            try
            {
                func(task, param.sharedParam);
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

        AGZ_ASSERT(workers_.empty());

        tasks_ = std::move(tasks);
        params_ = std::unique_ptr<Params>(new Params{ sharedParam, tasks_, taskMut_, exceptions_, exceptionMut_ });

        for(int i = 0; i < workerCount_; ++i)
            workers_.emplace_back(&Worker<Func>, func, *params_);
        Worker(func, *params_);

        for(auto &worker : workers_)
            worker.join();
        workers_.clear();

        return exceptions_.empty();
    }

    /**
     * @brief 将一组给定的任务分配给指定数量的线程后返回
     */
    template<typename Func>
    void RunAsync(Func &&func, const SharedParamType &sharedParam, std::queue<TaskType> tasks)
    {
        exceptions_.clear();

        AGZ_ASSERT(workers_.empty());

        tasks_ = std::move(tasks);
        params_ = std::unique_ptr<Params>(new Params{ sharedParam, tasks_, taskMut_, exceptions_, exceptionMut_ });

        for(int i = 0; i < workerCount_ + 1; ++i)
            workers_.emplace_back(&Worker<Func>, func, *params_);
    }

    /**
     * @brief 等待之前分配的任务完成
     * 
     * @return 在执行任务的途中是否发生了异常
     */
    bool Join()
    {
        for(auto &worker : workers_)
        {
            if(worker.joinable())
                worker.join();
        }
        workers_.clear();
        params_ = nullptr;
        return exceptions_.empty();
    }

    /**
     * @brief 同Join
     */
    bool Sync() { return Join(); }
    
    /**
     * @brief 直接终止之前分配的任务
     * @return 已执行的任务中是否发生了异常
     */
    bool Stop()
    {
        std::lock_guard<std::mutex> lk(taskMut_);
        tasks_.clear();
        return Join();
    }

    /**
     * @brief 之前分配的任务是否已经全部完成
     */
    bool IsCompleted() const
    {
        std::lock_guard<std::mutex> lk(taskMut_);
        return tasks_.empty();
    }

    /**
     * @brief 在之前已经执行的任务中是否发生了异常
     */
    bool HasException() const
    {
        std::lock_guard<std::mutex> lk(taskMut_);
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
