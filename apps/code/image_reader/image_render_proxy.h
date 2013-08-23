#ifndef IMAGE_RENDERER_H_
#define IMAGE_RENDERER_H_

#include "image_item.h"
#include "image_thumbnail.h"
#include "image_tasks.h"

namespace image
{

/// ImageRenderProxy manages:
/// 1. Generate render task
/// 2. Maintain the render settings
class ImageModel;
class ImageRenderProxy : public QObject
{
    Q_OBJECT
public:
    ImageRenderProxy();
    ~ImageRenderProxy();

    /// Update the render setting
    void updateRenderSetting(const RenderSetting & setting);

    /// Get the read-only render setting
    const RenderSetting& renderSetting() const { return render_setting_; }

    /// Get the read-write render setting
    RenderSetting& renderSetting() { return render_setting_; }

    /// Render a thumbnail image with givin rectangle
    void renderThumbnail(const int image_idx,
                         const QRect &rect,
                         ImageModel *model);

    /// Render a image with given setting
    /// current image means the main image displayed on screen
    /// destination image is the image to be rendered
    void renderImage(const int image_idx,
                     const RenderSetting &setting,
                     ImageModel *model);

    /// Prerender a image with given setting
    void prerenderImage(const int idx,
                        const RenderSetting &setting,
                        ImageModel *model);

private:
    /// global render setting
    RenderSetting render_setting_;
};

/// @brief image rendering task
class RenderTask : public BaseTask
{
public:
    RenderTask(shared_ptr<ImageItem> ptr,
               const RenderSetting & s,
               ImageModel *model,
               bool prerender)
        : image_(ptr)
        , model_(model)
        , key_(image_->name())
        , render_setting_(s)
        , prerender_(prerender)
    {}
    RenderTask(const ImageKey &k,
               ImageModel *model,
               const RenderSetting &s,
               bool prerender)
        : image_()
        , model_(model)
        , key_(k)
        , render_setting_(s)
        , prerender_(prerender)
    {}
    virtual ~RenderTask() {}

    void exec();

private:
    // estimate whether current task is valid
    bool isRenderingValid();

private:
    // instance of the image item that is created when executing this task
    shared_ptr<ImageItem> image_;

    // reference of the image renderer
    ImageModel *model_;

    // key of the image
    ImageKey key_;

    // instance of the render setting
    RenderSetting render_setting_;

    // is render or prerender
    bool prerender_;

    NO_COPY_AND_ASSIGN(RenderTask);
};

/// @brief Thumbnail Rendering Task
class ThumbnailRenderTask : public BaseTask
{
public:
    ThumbnailRenderTask(const QRect& rect,
                        const QRect& bounding_rect,
                        ImageModel *model,
                        shared_ptr<ImageItem> image);
    virtual ~ThumbnailRenderTask();

    void exec();

private:
    shared_ptr<ImageItem> image_;
    QRect    area_;
    QRect    bounding_rect_;
    ImageModel *model_;

    NO_COPY_AND_ASSIGN(ThumbnailRenderTask);
};

};

#endif
