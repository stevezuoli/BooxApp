#include "djvu_task.h"
#include "djvu_source.h"

namespace djvu_reader
{

DjVuRenderTask::DjVuRenderTask(DjVuSource * source, int page_num, const RenderSetting & render_setting, bool prerender)
: BaseTask(DJVU_RENDER)
, source_(source)
, page_num_(page_num)
, render_setting_(render_setting)
, prerender_(prerender)
{
}

DjVuRenderTask::~DjVuRenderTask()
{
}

void DjVuRenderTask::exec()
{
    start();
    if (!isRenderingValid())
    {
        // if it is an invalid prerender task
        abort();
        return;
    }

    DjVuPagePtr page = source_->getPage(page_num_);

    // decode
    source_->getPageDjvuImage(page_num_, false);

    // check whether decoding finished
    GP<DjVuFile> file = source_->getDjVuDoc()->get_djvu_file(page_num_);
    if (file == 0 || !file->is_decode_ok())
    {
        return;
    }

    // render the page
    if (page->render(source_, render_setting_))
    {
        // finish this task if render is done
        source_->notifyPageRenderReady(page);
        abort();
    }
}

bool DjVuRenderTask::isRenderingValid()
{
    if (render_setting_.isThumbnail() || source_->renderPolicy()->isRenderingPage(page_num_))
    {
        return true;
    }
    return false;
}


DjVuGetContentTask::DjVuGetContentTask(DjVuSource * source, int page_num)
: BaseTask(DJVU_GET_CONTENT)
, source_(source)
, page_num_(page_num)
{
}

DjVuGetContentTask::~DjVuGetContentTask()
{
}

void DjVuGetContentTask::exec()
{
    start();
    DjVuPagePtr page = source_->getPage(page_num_);

    // decode
    source_->getPageDjvuImage(page_num_, false);

    // check whether decoding finished
    GP<DjVuFile> file = source_->getDjVuDoc()->get_djvu_file(page_num_);
    if (file == 0 || !file->is_decode_ok())
    {
        return;
    }

    // get content area
    if (page->getContentArea(source_).isValid())
    {
        // finish this task if render is done
        source_->notifyPageContentAreaReady(page);
        abort();
    }
}

}
