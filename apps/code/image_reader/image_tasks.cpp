
#include "image_tasks.h"

using namespace ui;

namespace image
{

void BaseTask::fromImageStatus(ImageStatus s)
{
    switch (s)
    {
    case IMAGE_STATUS_WAIT:
        status_ = TASK_WAIT;
        break;

    case IMAGE_STATUS_PAUSE:
        status_ = TASK_PAUSE;
        break;

    case IMAGE_STATUS_DONE:
    case IMAGE_STATUS_ABORT:
    case IMAGE_STATUS_FAIL:
        status_ = TASK_STOP;
        break;

    case IMAGE_STATUS_RUNNING:
        status_ = TASK_RUN;
        break;

    default:
        status_ = TASK_ERROR;
        break;
    }
}

}
