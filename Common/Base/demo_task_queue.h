#ifndef DEMO_TASK_QUEUE_H_
#define DEMO_TASK_QUEUE_H_

#include <memory>

namespace DEMO {

class TaskRunable {
public:
    TaskRunable() = default;

    virtual ~TaskRunable() = default;

    virtual bool Run() = 0;
};

template <class ClosureType>
class ClosureTaskRunable : public TaskRunable {
public:
    explicit ClosureTaskRunable(ClosureType&& closure) : closure_(std::forward<ClosureType>(closure)) {
    }

private:
    bool Run() override {
        closure_();
        return true;
    }

    typename std::remove_const<typename std::remove_reference<ClosureType>::type>::type closure_;
};

class TaskQueue {
public:
    explicit TaskQueue(const char* queue_name = 0);

    ~TaskQueue();

    template <class ClosureType>
    void AsyncRun(ClosureType&& closure) {
        std::unique_ptr<TaskRunable> t(new ClosureTaskRunable<ClosureType>((std::forward<ClosureType>(closure))));
        PostTask(std::move(t));
    }

protected:
    void PostTask(std::unique_ptr<TaskRunable> task);

private:
    class Impl;

    std::shared_ptr<Impl> impl_;
};
}  // namespace DEMO

#endif  // DEMO_TASK_QUEUE_H_
