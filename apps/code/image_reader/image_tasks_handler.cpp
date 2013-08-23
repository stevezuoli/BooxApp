
#include "image_tasks_handler.h"

using namespace ui;

namespace image
{

ImageTasksHandler::ImageTasksHandler()
: tasks_()
, cur_task_(0)
, thread_running_(false)
{
}

ImageTasksHandler::~ImageTasksHandler()
{
    clearTasks();
}

ImageTasksHandler& ImageTasksHandler::instance()
{
    static ImageTasksHandler instance;
    return instance;
}

void ImageTasksHandler::addTask(BaseTask *t, bool append)
{
    if (append)
    {
        tasks_.append(t);
    }
    else
    {
        tasks_.clear();
        abortCurTask();
        tasks_.prepend(t);
    }

    if (!future_.isRunning())
    {
        startThread();
    }

}

void ImageTasksHandler::abortCurTask()
{
    if (cur_task_)
    {
        cur_task_->abort();
    }
}

void ImageTasksHandler::pauseCurTask()
{
    if (cur_task_)
    {
        cur_task_->pause();
    }
}

void ImageTasksHandler::startCurTask()
{
    if (cur_task_)
    {
        cur_task_->start();
    }
}

void ImageTasksHandler::execute()
{
    BaseTask *t = ImageTasksHandler::instance().currentTask();

    while (t != 0)
    {
        switch (t->getStatus())
        {
        case TASK_WAIT:
        case TASK_RUN:
            {
                // the current task is to be started or still running,
                // execute it.
                t->exec();
            }
            break;

        case TASK_PAUSE:
            // current task is pausing, it should be pushed to the
            // back of the task queue again; at the same time, a new
            // task is retrieved
            t->start();
            ImageTasksHandler::instance().addTask(t);
            ImageTasksHandler::instance().retrieveNextTask();
            break;

        case TASK_ERROR:
        case TASK_STOP:
            // current task has been finished, it should be removed
            // and then retrieve a new task
            ImageTasksHandler::instance().removeCurrentTask();
            ImageTasksHandler::instance().retrieveNextTask();
            break;

        default:
            break;
        }

#ifndef _WINDOWS
        // sleep for 100 miliseconds for GUI events
        usleep(1000 * 100);
#endif
        t = ImageTasksHandler::instance().currentTask();
    }
}

void ImageTasksHandler::removeCurrentTask()
{
    if (cur_task_ != 0 && cur_task_->getStatus() == TASK_RUN)
    {
        // if current task is still runnint, abort it
        cur_task_->abort();
    }

    // delete current task instance
    delete cur_task_;
    cur_task_ = 0;
}

void ImageTasksHandler::clearTasks()
{
    removeCurrentTask();
    tasks_.clear();
}

void ImageTasksHandler::retrieveNextTask()
{
    // NOTICE: current task MUST be handled before executing this
    // function
    // 1. Remove it
    // 2. Push it back to the tasks queue
    cur_task_ = tasks_.grab_front();
}

BaseTask* ImageTasksHandler::currentTask()
{
    if (cur_task_ == 0)
    {
        retrieveNextTask();
    }

    return cur_task_;
}

void ImageTasksHandler::startThread()
{
    future_ = QtConcurrent::run(ImageTasksHandler::execute);
}

void ImageTasksHandler::stopThread()
{
    if (future_.isRunning())
    {
        future_.cancel();
        future_.waitForFinished();
    }
}

void ImageTasksHandler::pauseThread()
{
    if (future_.isRunning())
    {
        future_.pause();
    }
}

void ImageTasksHandler::resumeThread()
{
    if (future_.isPaused())
    {
        future_.resume();
    }
}

}  // namespace image
