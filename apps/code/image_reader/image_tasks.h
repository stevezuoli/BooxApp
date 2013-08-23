#ifndef IMAGE_TASKS_H_
#define IMAGE_TASKS_H_

#include <QtCore/QMutexLocker>

#include "image_utils.h"

namespace image
{

enum TaskStatus
{
    TASK_WAIT = 0,
    TASK_RUN,
    TASK_PAUSE,
    TASK_STOP,
    TASK_ERROR
};

/// @brief Base class of the task
class BaseTask
{
public:
    BaseTask(): status_(TASK_WAIT) {}
    virtual ~BaseTask() {}

    // operations
    virtual void exec() = 0;

    void abort() {status_ = TASK_STOP;}
    void pause() {status_ = TASK_PAUSE;}
    void start() {status_ = TASK_RUN;}

    TaskStatus getStatus() {return status_;}

protected:
    void fromImageStatus(ImageStatus s);

private:
    // status of the task
    TaskStatus status_;

    NO_COPY_AND_ASSIGN(BaseTask);
};

/// @brief TasksQueue is a container(queue) of all the tasks
/// it is not thread-safe.
template<class T>
class TasksQueue
{
public:
    TasksQueue():queue_(), mutex_() {};
    ~TasksQueue() {clear();}

    void append(T t)
    {
        QMutexLocker mtx(&mutex_);
        queue_.push_back(t);
    }

    void prepend(T t)
    {
        QMutexLocker mtx(&mutex_);
        queue_.push_front(t);
    }

    void clear()
    {
        QMutexLocker mtx(&mutex_);
        QueueIter begin = queue_.begin();
        QueueIter end   = queue_.end();
        QueueIter iter  = begin;

        for(; iter != end; ++iter)
        {
            // delete left tasks
            delete *iter;
        }

        queue_.clear();
    }

    T grab_front()
    {
        QMutexLocker mtx(&mutex_);
        if (size() > 0)
        {
            T t = queue_.front();
            queue_.pop_front();
            return t;
        }
        return 0;
    }

    T grab_back()
    {
        QMutexLocker mtx(&mutex_);
        if (size() > 0)
        {
            T t = queue_.back();
            queue_.pop_back();
            return t;
        }
        return 0;
    }

private:
    typedef list<T> Queue;
    typedef typename Queue::iterator QueueIter;

private:
    int size()
    {
        return static_cast<int>(queue_.size());
    }

private:
    // tasks queue
    Queue queue_;

    // mutex
    QMutex mutex_;

    NO_COPY_AND_ASSIGN(TasksQueue);
};

};  // namespace image

#endif
