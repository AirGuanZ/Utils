#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace AGZ {

/**
 * @brief �Բ������κι��������������䣬���Դ���Ϊdummy parameter
 */
inline struct NoSharedParam_t { } NO_SHARED_PARAM;
    
/**
 * @brief ��̬���������
 * 
 * ��һ���������������һ���������߳����
 * 
 * �ڴ˹����в��ɶ�̬��ӻ�ɾ����������֮���˳��Ͳ�����Ҳ���õ��κα�֤
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
     * @param workerCount �������Ĺ����߳�������Ϊ������ʱʹ��Ӳ���߳���
     */
    explicit StaticTaskDispatcher(int workerCount)
    {
        if(workerCount <= 0)
            workerCount = std::thread::hardware_concurrency();
        workerCount_ = (std::max)(1, workerCount) - 1;
    }

    /**
     * @brief ��ɸ�����һ�����񣬻����֮ǰ����ʱ�������쳣�б�
     * 
     * @return ���������Ĺ�����û���׳��κ��쳣���򷵻�true�����򷵻�false��
     *         ����falseʱ��ͨ�� GetExceptions �鿴�쳣�б�
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
     * @brief ȡ����һ�ε��� Run ʱ�������쳣�б�
     */
    const std::vector<std::exception> &GetExceptions() const
    {
        return exceptions_;
    }
};

} // namespace AGZ
