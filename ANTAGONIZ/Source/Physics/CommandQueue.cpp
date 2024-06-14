#include "CommandQueue.hpp"

namespace Ge
{

    void CommandQueue::push(Command* command)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(command);
        cv_.notify_one();
    }

    Command* CommandQueue::pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        Command* command = queue_.front();
        queue_.pop();
        return command;
    }

    bool CommandQueue::empty() 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
}