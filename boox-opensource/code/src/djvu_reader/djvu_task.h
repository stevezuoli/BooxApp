#ifndef DJVU_TASK_H_
#define DJVU_TASK_H_

#include "djvu_utils.h"

using namespace vbf;
namespace djvu_reader
{

enum TaskType
{
    DJVU_RENDER = 0,
    DJVU_GET_CONTENT,
    DJVU_SEARCH
};

class DjVuSource;
class DjVuRenderTask : public BaseTask
{
public:
    DjVuRenderTask(DjVuSource * source, int page_num, const RenderSetting & render_setting, bool prerender = false);
    virtual ~DjVuRenderTask();
    virtual void exec();

private:
    bool isRenderingValid();

private:
    DjVuSource*    source_;
    int            page_num_;
    RenderSetting  render_setting_;
    bool           prerender_;
};

class DjVuGetContentTask : public BaseTask
{
public:
    DjVuGetContentTask(DjVuSource * source, int page_num);
    virtual ~DjVuGetContentTask();
    virtual void exec();

private:
    DjVuSource*    source_;
    int            page_num_;
};

};

#endif
