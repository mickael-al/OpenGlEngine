#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <tuple>
#include <utility>

class ThreadPool {
public:
    ThreadPool()
        : stop(false)
    {
        m_nThreads = std::thread::hardware_concurrency();
        m_nThreads = (m_nThreads == 0) ? 4 : m_nThreads;

        for (unsigned int i = 0; i < m_nThreads; ++i) 
        {
            workers.emplace_back([this] {
                while (true) {
                    TaskBase* task = nullptr;

                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                            });

                        if (this->stop && this->tasks.empty())
                            return;

                        task = tasks.front();
                        tasks.pop();
                    }

                    task->execute();
                    delete task;
                }
                });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }

        condition.notify_all();
        for (std::thread& t : workers)
            if (t.joinable())
                t.join();
    }

    template<typename Func, typename... Args>
    void enqueue(Func* f, Args&&... args)
    {
        auto* task = new Task<Func, Args...>(f, std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(task);
        }
        condition.notify_one();
    }

    inline unsigned int getNbThread() const
    {
        return m_nThreads;
    }

private:
    struct TaskBase {
        virtual ~TaskBase() = default;
        virtual void execute() = 0;
    };

    template<typename Func, typename... Args>
    struct Task : TaskBase {
        Func* f;
        std::tuple<Args...> args;

        Task(Func* func, Args&&... a)
            : f(func), args(std::forward<Args>(a)...) {}

        void execute() override {
            callFunc(std::index_sequence_for<Args...>{});
        }

        template<std::size_t... I>
        void callFunc(std::index_sequence<I...>) {
            (*f)(std::get<I>(args)...);
        }
    };

    unsigned int m_nThreads;
    std::vector<std::thread> workers;
    std::queue<TaskBase*> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

#endif // !__THREAD_POOL__