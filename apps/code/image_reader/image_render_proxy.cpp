#include "image_render_proxy.h"

#ifndef _WINDOWS
#include <inttypes.h>
#endif

#include "image_model.h"
#include "image_tasks_handler.h"
#include "image_global_settings.h"

using namespace ui;
using namespace std;

namespace image
{
void getThumbnailRectangle(const QRect& bounding_rect,
                           const QSize& origin_size,
                           QRect* thumb_rect)
{
    int x = 0;
    int y = 0;
    int thumb_width = 0;
    int thumb_height = 0;
    if (bounding_rect.width() >= origin_size.width() &&
        bounding_rect.height() >= origin_size.height())
    {
        // if the page's actual area is smaller than the rectangle
        // the original page would be rendered
        thumb_width = origin_size.width();
        thumb_height = origin_size.height();
    }
    else
    {
        // otherwise, fit to the rectangle
        double zoom = min(static_cast<double>(bounding_rect.width()) /
            static_cast<double>(origin_size.width())
            , static_cast<double>(bounding_rect.height()) /
            static_cast<double>(origin_size.height()));
        thumb_width = origin_size.width() * zoom;
        thumb_height = origin_size.height() * zoom;
    }

    x = bounding_rect.left() + ((bounding_rect.width() - thumb_width) >> 1);
    y = bounding_rect.top() + ((bounding_rect.height() - thumb_height) >> 1);
    thumb_rect->setTopLeft(QPoint(x, y));
    thumb_rect->setWidth(thumb_width);
    thumb_rect->setHeight(thumb_height);
}

bool isGlobalSettingChanged(const ImageItem * image)
{
    return (image->dithered() != ImageGlobalSettings::instance().needDither() ||
            image->converted() != ImageGlobalSettings::instance().needConvert() ||
            image->format() != ImageGlobalSettings::instance().convertFormat() ||
            image->smoothed() != ImageGlobalSettings::instance().needSmooth());
}

ImageRenderProxy::ImageRenderProxy()
{
}

ImageRenderProxy::~ImageRenderProxy()
{
}

void ImageRenderProxy::renderThumbnail(const int image_idx,
                                       const QRect &rect,
                                       ImageModel *model)
{
    assert(model != 0);

    // 1. get the image by idx
    shared_ptr<ImageItem> image(model->getImage(image_idx));
    if (image.get() == NULL)
    {
        qDebug("Thumbnail: get an unknown image\n");
        return;
    }

    // 3. try to retrieve the thumbnail image, if it has been
    // rendered, just return it
    ImageKey name;
    if (!model->getImageNameByIndex(image_idx, name))
    {
        return;
    }

    shared_ptr<ImageThumbnail> thumb(model->getThumbsMgr().getImage(name));
    QRect thumb_rect;
    if (thumb.get() != 0 && thumb->image() != 0)
    {
        // calculate the display area of the thumbnail image
        QSize size = image->actualSize();
        if (size.isValid())
        {
            getThumbnailRectangle(rect, size, &thumb_rect);
            if (thumb->image()->size() == thumb_rect.size())
            {
                // Notify client that the image is ready.
                thumb->updateDisplayArea(thumb_rect);
                model->onThumbnailReady(thumb, rect);
                return;
            }
            else
            {
                thumb->clearPage();
            }
        }
    }

    // 4. Prepend a thumbnail rendering task
    ThumbnailRenderTask* task = new ThumbnailRenderTask(thumb_rect, rect, model, image);
    ImageTasksHandler::instance().addTask(task, true);
}

void ImageRenderProxy::renderImage(const int image_idx,
                                   const RenderSetting &setting,
                                   ImageModel *model)
{
    assert(model != 0);
    if (image_idx < 0 || image_idx >= model->imageCount())
    {
        return;
    }

    shared_ptr<ImageItem> image(model->getImage(image_idx));
    RenderTask *render_task = 0;
    if (image == 0)
    {
        // the wanted image is not cached, so update the render setting and
        // rerender it
        qDebug("A unknown image, why?\n");
        ImageKey key;
        if (model->getImageNameByIndex(image_idx, key))
        {
            render_task = new RenderTask(key, model, setting, false);
        }
    }
    else
    {
        if (image->image() != 0 && setting == image->renderSetting() && !isGlobalSettingChanged(image. get()))
        {
            model->onImageReady(image, IMAGE_STATUS_DONE, true);
        }
        else
        {
            render_task = new RenderTask(image, setting, model, false);
        }
    }

    if (render_task)
    {
        // prepend to the head of the task queue
        ImageTasksHandler::instance().addTask(render_task, false);
    }
}

void ImageRenderProxy::prerenderImage(const int idx,
                                      const RenderSetting &setting,
                                      ImageModel *model)
{
    assert(model != 0);

    if (idx < 0 || idx >= model->imageCount())
    {
        return;
    }

    RenderTask *render_task = 0;
    shared_ptr<ImageItem> image(model->getImage(idx));
    if (image.get() == NULL)
    {
        // the wanted page is not cached, so update the render setting and
        // rerender it.
        qDebug("A unknown image, why?\n");
        ImageKey key;
        if (model->getImageNameByIndex(idx, key))
        {
            render_task = new RenderTask(key, model, setting, true);
        }
    }
    else
    {
        render_task = new RenderTask(image, setting, model, true);
    }

    if (render_task)
    {
        // prepend to the head of the task queue
        ImageTasksHandler::instance().addTask(render_task, true);
    }
}

void ImageRenderProxy::updateRenderSetting(const RenderSetting &setting)
{
    render_setting_ = setting;
}

// Render Task Implementation -------------------------------------------------
void RenderTask::exec()
{
    assert(model_ != 0);

    ImageStatus image_status = IMAGE_STATUS_RUNNING;
    fromImageStatus(image_status);

    if (!isRenderingValid())
    {
        // if it is an invalid prerender task
        abort();
        return;
    }

    if (image_.get() == NULL)
    {
        // get the image_
        // at first check whether the same page has been cached
        image_ = model_->getImage(key_);

        if (image_.get() == NULL)
        {
            // the dstination page is not generated yet.
            // get the page by number
            image_ = model_->addImage(key_);
        }
    }

    if (image_.get())
    {
        image_status = image_->render(render_setting_, model_);
    }
    else
    {
        image_status = IMAGE_STATUS_ABORT;
    }

    // do not notify the other listeners if it is prerender task
    model_->onImageReady(image_, image_status, (!prerender_));
    fromImageStatus(image_status);
}

bool RenderTask::isRenderingValid()
{
    int page_number = model_->getIndexByImageName(key_);
    if (model_->renderPolicy()->isRenderingPage(page_number))
    {
        return true;
    }
    return false;
}

// implementation of thumbnail render task-------------------------------------
ThumbnailRenderTask::ThumbnailRenderTask(const QRect& rect,
                                         const QRect& bounding_rect,
                                         ImageModel *model,
                                         shared_ptr<ImageItem> image)
  : image_(image)
  , area_(rect)
  , bounding_rect_(bounding_rect)
  , model_(model)
{
}

ThumbnailRenderTask::~ThumbnailRenderTask()
{
}

void ThumbnailRenderTask::exec()
{
    // check whether the thumbnail is ready
    ImageStatus image_status = IMAGE_STATUS_RUNNING;
    shared_ptr<ImageThumbnail> thumb(model_->getThumbsMgr().getImage(image_->name()));

    if (thumb.get() == 0)
    {
        thumb.reset(new ImageThumbnail(image_->name()));
        thumb->setKey(image_->index());
        model_->getThumbsMgr().addImage(thumb->path(), thumb);
    }

    image_status = image_->renderThumbnail(bounding_rect_,
                                           area_,
                                           thumb,
                                           model_);

    if (image_status == IMAGE_STATUS_DONE)
    {
        thumb->updateDisplayArea(area_);
        model_->onThumbnailReady(thumb, bounding_rect_);
    }

    fromImageStatus(image_status);
}

}
