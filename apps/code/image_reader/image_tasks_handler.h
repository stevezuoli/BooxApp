
#ifndef IMAGE_TASKS_HANDLER_H_
#define IMAGE_TASKS_HANDLER_H_

#include "image_tasks.h"

using namespace ui;

namespace image
{
/// @brief ImageTasksHandler is used for executing the tasks which might
/// consume mess of time:

/// NOTICE: this class is not thread-safe
class ImageTasksHandler : public QObject
{
    Q_OBJECT

private:
    ImageTasksHandler();
    ImageTasksHandler(const ImageTasksHandler&);

public:
    ~ImageTasksHandler();

    static ImageTasksHandler& instance();

    /// Start the thread
    void startThread();

    /// Pause the thread
    void pauseThread();

    /// Resume the thread
    void resumeThread();

    /// Stop the thread
    void stopThread();

    /// Clear the tasks queue and current task
    void clearTasks();

    /// Add a new task into the queue
    /// if the timer stops when adding, start it
    void addTask(BaseTask *t, bool append = true);

    /// Abort current task
    /// if there is no more task, stop the timer
    /// otherwise execute the next task
    void abortCurTask();

    /// Pause current task
    /// if there is no more task, stop the timer
    /// otherwise execute the next task
    void pauseCurTask();

    /// Start current task
    /// if the task has been paused beforehand, restart it
    void startCurTask();

private:
    /// Task executing function
    static void execute();

private:
    /// remove current task
    void removeCurrentTask();

    /// retrieve next task in the queue
    void retrieveNextTask();

    /// get current task
    BaseTask* currentTask();

private:
    /// task queue
    TasksQueue<BaseTask*> tasks_;

    /// reference to current executing task
    BaseTask* cur_task_;

    /// flag indicating whether the thread is running
    bool thread_running_;

    /// future of the thread
    QFuture<void> future_;
};

};  // namespace pdf

#endif
