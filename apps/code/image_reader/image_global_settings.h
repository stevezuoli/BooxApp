#ifndef IMAGE_GLOBAL_SETTINGS_H_
#define IMAGE_GLOBAL_SETTINGS_H_

#include "image_utils.h"

namespace image
{
class ImageGlobalSettings
{
private:
    ImageGlobalSettings()
        : need_dither_(true)
        , need_convert_(false)
        , convert_format_(QImage::Format_RGB16)
        , need_smooth_(false)
    {}

    NO_COPY_AND_ASSIGN(ImageGlobalSettings);

public:
    static ImageGlobalSettings & instance()
    {
        static ImageGlobalSettings settings;
        return settings;
    }

    void needDither(bool d) { need_dither_ = d; }
    bool needDither() { return need_dither_; }

    void needConvert(bool c) { need_convert_ = c; }
    bool needConvert() { return need_convert_; }

    void setConvertFormat(const QImage::Format f) { convert_format_ = f;}
    QImage::Format convertFormat() { return convert_format_; }

    void needSmooth(bool s) { need_smooth_ = s; }
    bool needSmooth() { return need_smooth_; }

private:
    bool need_dither_;
    bool need_convert_;
    QImage::Format convert_format_;
    bool need_smooth_;
};

};

#endif
