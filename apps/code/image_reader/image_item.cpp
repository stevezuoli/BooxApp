#include "image_item.h"

#ifndef _WINDOWS
#include <inttypes.h>
#endif

#include "floyd_steinberg_grayscale.h"
#include "floyd_steinberg_dithering.h"
#include "image_model.h"
#include "image_global_settings.h"
#include "image_render_policy.h"

using namespace vbf;
namespace image
{

static const double RELOAD_THRESHOLD = 2.0f;
scoped_ptr<DitheringStrategy> ImageItem::dithering_strategy_;

unsigned int tryCalcImageLength(const int width,
                                const int height,
                                QImage::Format f)
{
    unsigned int length = 0;
    switch (f)
    {
    case QImage::Format_Indexed8:
        {
            length = width * height;
        }
        break;
    default:
        break;
    }
    return length;
}

int getRealOrientation(RotateDegree rotate_degree)
{
    int real_orient = 0;
    switch (rotate_degree)
    {
    case ROTATE_0_DEGREE:
        real_orient = 0;
        break;
    case ROTATE_180_DEGREE:
        real_orient = 180;
        break;
    case ROTATE_270_DEGREE:
        real_orient = 270;
        break;
    case ROTATE_90_DEGREE:
        real_orient = 90;
        break;
    default:
        break;
    }
    return real_orient;
}

ImageItem::ImageItem(const ImageKey &k)
    : render_setting_()
    , private_info_()
    , locked_(false)
    , valid_(true)
    , actual_size_()
    , file_size_(0)
    , render_status_(IMAGE_STATUS_WAIT)
    , data_(0)
    , dirty_(false)
{
    if (dithering_strategy_ == 0)
    {
        dithering_strategy_.reset(new FloydSteinbergDithering(256));
    }
    private_info_.key = k;
}

ImageItem::~ImageItem()
{
}

int ImageItem::length()
{
    if (renderStatus() == IMAGE_STATUS_DONE)
    {
        assert(data_);
        return data_->numBytes();
    }
    return 0;
}

void ImageItem::setRenderSetting(const RenderSetting &setting)
{
    renderSetting() = setting;
}

void ImageItem::access(ImageModel *model)
{
    if (!conf_.accessed)
    {
        // load the configuration data
        if ( !vbf::loadDocumentOptions( model->contentManager(), private_info_.key, conf_.conf ) )
        {
            qWarning("Cannot load or create configuration node");
            return;
        }
        else
        {
            conf_.accessed = true;
        }

        // The progress should always be 1/1.
        QString progress("%1 / %2");
        conf_.conf.info.mutable_progress() = progress.arg(1).arg(1);
    }

    conf_.conf.info.updateLastAccess();
    conf_.conf.info.mutable_read_count() = conf_.conf.info.read_count() + 1;
}

void ImageItem::saveAccessRecord(ImageModel *model)
{
    if (conf_.accessed)
    {
        vbf::saveDocumentOptions( model->contentManager(), private_info_.key, conf_.conf );
    }
}

bool ImageItem::reload()
{
    // destroy the old image instance
    clearPage();

    //qDebug("Begin Load Image %s!\n", name().toStdString().c_str());
    // Reconstruct the image by key
    if (name() == EMPTY_BACKGROUND)
    {
        data_.reset(new QImage(EMPTY_BACKGROUND_WIDTH,
                               EMPTY_BACKGROUND_HEIGHT,
                               QImage::Format_ARGB32));
        QColor white(255, 255, 255);
        data_->fill(white.rgba());
    }
    else
    {
        data_.reset(new QImage(name()));
    }
    //qDebug("End Load Image! %s\n", name().toStdString().c_str());

    if (data_->isNull())
    {
        // if the image is broken, display the warning map
        data_->load(":/images/invalid.png");
    }

    private_info_.reset();
    dirty_    = false;
    actualSize() = data_->size();

    // reset the render setting
    // TODO. Use default rotation value
    RenderSetting setting;
    setting.setContentArea(QRect(0, 0, actualSize().width(), actualSize().height()));
    setting.setRotation(ROTATE_0_DEGREE);
    setRenderSetting(setting);
    return true;
}

void ImageItem::clearPage()
{
    setRenderStatus(IMAGE_STATUS_WAIT);
    data_.reset();
}

bool ImageItem::needReload(const RenderSetting &setting)
{
    // check the data_ is valid or not
    if (!data_.get())
    {
        return true;
    }

    // check wether current image is dirty or not
    if (dirty_)
    {
        int cur_area_size = data_->width() * data_->height();
        int scaled_area_size = setting.contentArea().width() *
            setting.contentArea().height();
        if (scaled_area_size > (static_cast<double>(cur_area_size) * RELOAD_THRESHOLD) ||
            scaled_area_size < (static_cast<double>(cur_area_size) / RELOAD_THRESHOLD))
        {
            return true;
        }
    }

    // if the global render setting changes, reload the image
    return false;
}

ImageStatus ImageItem::renderThumbnail(const QRect &bounding_rect,
                                       QRect &display_area,
                                       shared_ptr<ImageThumbnail> thumbnail,
                                       ImageModel *model)
{
    // check the previous thumbnail image
    if (thumbnail->image() != 0)
    {
        if (thumbnail->size() != display_area.size())
        {
            thumbnail->clearPage();
        }
        else
        {
            return IMAGE_STATUS_DONE;
        }
    }

    // if there is no data being loaded, reload the image
    scoped_ptr<QImage> cur_data;
    if (data_ == 0)
    {
        reload();
    }
    cur_data.reset(new QImage(*data_));

    if (!display_area.isValid())
    {
        getThumbnailRectangle(bounding_rect, cur_data->size(), &display_area);
    }

    if (model->getThumbsMgr().makeEnoughMemory(tryCalcImageLength(
                                               display_area.width(),
                                               display_area.height(),
                                               QImage::Format_Indexed8),
                                               name(),
                                               model->renderPolicy()))
    {
        int width  = display_area.width();
        int height = display_area.height();

        int cur_area_size = cur_data->width() * cur_data->height();
        int scaled_area_size = width * height;
        bool dithered = false;
        if (cur_area_size < scaled_area_size)
        {
            // pre-dither
            dithering_strategy_->dither(cur_data.get());
            dithered = true;
        }

        if (cur_data->size() != display_area.size())
        {
            cur_data.reset(scaled(display_area.size(), cur_data.get()));
        }

        if (!dithered && ImageGlobalSettings::instance().needDither())
        {
            // post-dither
            dithering_strategy_->dither(cur_data.get());
        }

        thumbnail->setImage(cur_data.release());
        thumbnail->setOriginSize(actualSize());
        return IMAGE_STATUS_DONE;
    }
    return IMAGE_STATUS_FAIL;
}

ImageStatus ImageItem::render(const RenderSetting &setting, ImageModel *model)
{
    if (renderSetting() == setting && renderStatus() == IMAGE_STATUS_DONE)
    {
        qDebug("Image:%s is already ready\n", name().toStdString().c_str());
        return renderStatus();
    }

    // MUST NOT reload at this time
    setRenderStatus(IMAGE_STATUS_WAIT);
    qDebug("Render Image:%s!!! \n", name().toStdString().c_str());

    if (!(renderSetting() == setting) &&
        !model->getImagesMgr().makeEnoughMemory(tryCalcImageLength(
                                                setting.contentArea().width(),
                                                setting.contentArea().height(),
                                                QImage::Format_Indexed8),
                                                name(),
                                                model->renderPolicy()))
    {
        setRenderStatus(IMAGE_STATUS_ABORT);
        return renderStatus();
    }

    // if the data is null, reload the image
    if (needReload(setting) && !reload())
    {
        qWarning("Reload fails\n\n");
        setRenderStatus(IMAGE_STATUS_ABORT);
        return renderStatus();
    }

    if (data_ == 0)
    {
        setRenderStatus(IMAGE_STATUS_ABORT);
        return renderStatus();
    }

    int cur_area_size = data_->width() * data_->height();
    int scaled_area_size = setting.contentArea().width() * setting.contentArea().height();
    if (cur_area_size < scaled_area_size)
    {
        // pre-quantize
        quantize();
    }
    else if (cur_area_size > scaled_area_size)
    {
        dirty_ = true;
    }

    if (setting.rotation() != renderSetting().rotation())
    {
        rotate(setting.rotation());
    }

    if (setting.contentArea() != renderSetting().contentArea())
    {
        if (!scaled(setting.contentArea().size()))
        {
            setRenderStatus(IMAGE_STATUS_ABORT);
            return renderStatus();
        }
    }

    // post-quantize
    quantize();

    setRenderSetting(setting);
    setRenderStatus(IMAGE_STATUS_DONE);
    return renderStatus();
}

void ImageItem::resetData(const QImage &new_data)
{
    clearPage();
    data_.reset(new QImage(new_data));
}

bool ImageItem::scaled(const QSize &size)
{
    if (data_ != 0)
    {
        QImage img = data_->scaled(size);
        resetData(img);
        return true;
    }
    return false;
}

QImage* ImageItem::scaled(const QSize &size, QImage *input)
{
    QImage result = input->scaled(size);
    QImage *output = new QImage(result);
    return output;
}

void ImageItem::rotate(const RotateDegree orient)
{
    if (data_ != 0)
    {
        int real_orient = getRealOrientation(orient);
        int prev_orient = getRealOrientation(renderSetting().rotation());
        real_orient = (real_orient - prev_orient + 360) % 360;

        QMatrix matrix;
        matrix.rotate(static_cast<qreal>(real_orient));
        QImage img = data_->transformed(matrix);
        resetData(img);
    }
}

void ImageItem::convert(const QImage::Format f)
{
    if (data_ != 0)
    {
        *(data_.get()) = data_->convertToFormat(f);
        private_info_.converted = true;
        private_info_.format = f;
    }
}

void ImageItem::quantize()
{
    if (ImageGlobalSettings::instance().needConvert() &&
        (!private_info_.converted || private_info_.format !=
        ImageGlobalSettings::instance().convertFormat()))
    {
        convert(ImageGlobalSettings::instance().convertFormat());
    }

    if (ImageGlobalSettings::instance().needDither() &&
        !private_info_.dithered)
    {
        // TODO. Remove color coverting from dithering.
        dithering_strategy_->dither(data_.get());
        private_info_.dithered = true;
    }

    if (ImageGlobalSettings::instance().needSmooth() &&
        !private_info_.smoothed)
    {
        // TODO. Add implementation here
        private_info_.smoothed = true;
    }
}

}
