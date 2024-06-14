#ifndef __ENGINE_COMMAND_QUEUE__
#define __ENGINE_COMMAND_QUEUE__

#include <functional>
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>
#include <future>

namespace Ge
{

    class Command
    {
    public:
        virtual void execute() = 0;
    };

    class CommandQueue final
    {
    public:
        void push(Command* command);
        Command* pop();
        bool empty();
    private:
        std::queue<Command*> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };

    template <typename T, typename ReturnType, typename... Args>
    class MethodCommandReturn : Command 
    {
    public:
        using MethodType = ReturnType(T::*)(Args...);

        MethodCommandReturn(T* object, MethodType method, std::promise<ReturnType> * promise, Args... args)
            : object_(object), method_(method), promise_(promise), args_(std::make_tuple(args...)) {}

        void execute() override
        {
            ReturnType result = apply(std::index_sequence_for<Args...>{});
            promise_->set_value(result);
        }

    private:
        template <std::size_t... Is>
        ReturnType apply(std::index_sequence<Is...>) {
            return (object_->*method_)(std::get<Is>(args_)...);
        }

        T* object_;
        MethodType method_;
        std::tuple<Args...> args_;
        std::promise<ReturnType> * promise_;
    };

    template <typename T, typename... Args>
    class MethodCommand : Command
    {
    public:
        using MethodType = void(T::*)(Args...);

        MethodCommand(T* object, MethodType method, Args... args)
            : object_(object), method_(method), args_(std::make_tuple(args...)) {}


        void execute() override
        {
            apply(std::index_sequence_for<Args...>{});
        }

    private:
        template <std::size_t... Is>
        void apply(std::index_sequence<Is...>) 
        {
           (object_->*method_)(std::get<Is>(args_)...);
        }

        T* object_;
        MethodType method_;
        std::tuple<Args...> args_;
    };
}
#endif // !__ENGINE_COMMAND_QUEUE__