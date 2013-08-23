#include "djvu_page.h"
#include "djvu_source.h"

namespace djvu_reader
{

static QVector<QRgb> COLOR_TABLE;

static void initialColorTable()
{
    if (COLOR_TABLE.size() <= 0)
    {
        for(int i = 0; i < 256; ++i)
        {
            COLOR_TABLE.push_back(qRgba(i, i, i, 255));
        }
    }
}

static void qRect2GRect(const QRect & qrect, GRect & grect)
{
    grect.xmin = qrect.left();
    grect.ymin = qrect.top();
    grect.xmax = qrect.left() + qrect.width();
    grect.ymax = qrect.top() + qrect.height();
}

static void gRect2QRect(const GRect & grect, QRect & qrect)
{
    if (grect.isempty())
    {
        qrect = QRect();
    }
    else
    {
        qrect.setLeft(grect.xmin);
        qrect.setTop(grect.ymin);
        qrect.setWidth(grect.width());
        qrect.setHeight(grect.height());
    }
}

static void fmt_convert_row(const GPixel *p, int w, char *buf)
{
    const uint32_t (*r)[256] = GlobalRenderFormat::instance().rgb;
    const uint32_t xorval = GlobalRenderFormat::instance().xorval;
    switch(GlobalRenderFormat::instance().format_style)
    {
    case DDJVU_FORMAT_BGR24:    /* truecolor 24 bits in BGR order */
        {
            memcpy(buf, (const char*)p, 3 * w);
            break;
        }
    case DDJVU_FORMAT_RGB24:    /* truecolor 24 bits in RGB order */
        { 
            while (--w >= 0)
            {
                buf[0] = p->r; buf[1] = p->g; buf[2] = p->b; 
                buf += 3;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_RGBMASK16: /* truecolor 16 bits with masks */
        {
            unsigned short *b = (unsigned short*)buf;
            while (--w >= 0)
            {
                b[0]=(r[0][p->r]|r[1][p->g]|r[2][p->b])^xorval;
                b += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_RGBMASK32: /* truecolor 32 bits with masks */
        {
            unsigned int *b = (unsigned int*)buf;
            while (--w >= 0)
            {
                b[0]=(r[0][p->r]|r[1][p->g]|r[2][p->b])^xorval; 
                b += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_GREY8:    /* greylevel 8 bits */
        {
            while (--w >= 0)
            {
                buf[0]=(5*p->r + 9*p->g + 2*p->b)>>4; 
                buf += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_PALETTE8: /* paletized 8 bits (6x6x6 color cube) */
        {
            const unsigned int *u = GlobalRenderFormat::instance().palette;
            while (--w >= 0)
            {
                buf[0] = u[r[0][p->r]+r[1][p->g]+r[2][p->b]]; 
                buf += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_MSBTOLSB: /* packed bits, msb on the left */
        {
            unsigned char s = 0, m = 0x80;
            while (--w >= 0)
            {
                if ( 5*p->r + 9*p->g + 2*p->b < 0xc00 ) { s |= m; }
                if (! (m >>= 1)) { *buf++ = s; s = 0; m = 0x80; }
                p += 1;
            }
            if (m < 0x80) { *buf++ = s; }
            break;
        }
    case DDJVU_FORMAT_LSBTOMSB: /* packed bits, lsb on the left */
        {
            unsigned char s = 0, m = 0x1;
            while (--w >= 0)
            {
                if ( 5*p->r + 9*p->g + 2*p->b < 0xc00 ) { s |= m; }
                if (! (m <<= 1)) { *buf++ = s; s = 0; m = 0x1; }
                p += 1;
            }
            if (m > 0x1) { *buf++ = s; }
            break;
        }
    }
}

static void
fmt_convert(GP<GPixmap> pm, char *buffer, int rowsize)
{
    int w = pm->columns();
    int h = pm->rows();

    // Loop on rows
    if (GlobalRenderFormat::instance().rtoptobottom)
    {
        for(int r = h-1; r >= 0; r--, buffer += rowsize)
        {
            fmt_convert_row((*pm)[r], w, buffer);
        }
    }
    else
    {
        for(int r = 0; r < h; r++, buffer += rowsize)
        {
            fmt_convert_row((*pm)[r], w, buffer);
        }
    }
}

static void fmt_convert_row(unsigned char *p, unsigned char *g, int w, char *buf)
{
    const unsigned int (*r)[256] = GlobalRenderFormat::instance().rgb;
    const unsigned int xorval = GlobalRenderFormat::instance().xorval;
    switch(GlobalRenderFormat::instance().format_style)
    {
    case DDJVU_FORMAT_BGR24:    /* truecolor 24 bits in BGR order */
    case DDJVU_FORMAT_RGB24:    /* truecolor 24 bits in RGB order */
        {
            while (--w >= 0)
            {
                buf[0] = buf[1] = buf[2] = g[*p];
                buf += 3;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_RGBMASK16: /* truecolor 16 bits with masks */
        {
            unsigned short *b = (unsigned short*)buf;
            while (--w >= 0)
            {
                unsigned char x = g[*p];
                b[0]=(r[0][x]|r[1][x]|r[2][x])^xorval; 
                b += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_RGBMASK32: /* truecolor 32 bits with masks */
        {
            unsigned int *b = (unsigned int*)buf;
            while (--w >= 0)
            {
                unsigned char x = g[*p];
                b[0]=(r[0][x]|r[1][x]|r[2][x])^xorval; 
                b += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_GREY8:    /* greylevel 8 bits */
        {
            while (--w >= 0)
            {
                buf[0]=g[*p];
                buf += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_PALETTE8: /* paletized 8 bits (6x6x6 color cube) */
        {
            const unsigned int *u = GlobalRenderFormat::instance().palette;
            while (--w >= 0)
            {
                buf[0] = u[g[*p]*(1+6+36)];
                buf += 1;
                p += 1;
            }
            break;
        }
    case DDJVU_FORMAT_MSBTOLSB: /* packed bits, msb on the left */
        {
            unsigned char s=0, m=0x80;
            while (--w >= 0)
            {
                if (g[*p] < 0xc0) { s |= m; }
                if (! (m >>= 1)) { *buf++ = s; s = 0; m = 0x80; }
                p += 1;
            }
            if (m < 0x80) { *buf++ = s; }
            break;
        }
    case DDJVU_FORMAT_LSBTOMSB: /* packed bits, lsb on the left */
        {
            unsigned char s = 0, m = 0x1;
            while (--w >= 0)
            {
                if (g[*p] < 0xc0) { s |= m; }
                if (! (m <<= 1)) { *buf++ = s; s = 0; m = 0x1; }
                p += 1;
            }
            if (m > 0x1) { *buf++ = s; }
            break;
        }
    }
}

static void fmt_convert(GP<GBitmap> bm, char * buffer, int rowsize)
{
    int w = bm->columns();
    int h = bm->rows();
    int m = bm->get_grays();

    // Gray levels
    int i;
    unsigned char g[256];
    for (i = 0; i < m; i++)
    {
        g[i] = 255 - ( i * 255 + (m - 1)/2 ) / (m - 1);
    }

    for (i=m; i<256; i++)
    {
        g[i] = 0;
    }

    // Loop on rows
    if (GlobalRenderFormat::instance().rtoptobottom)
    {
        for(int r = h-1; r >= 0; r--, buffer += rowsize)
        {
            fmt_convert_row((*bm)[r], g, w, buffer);
        }
    }
    else
    {
        for(int r = 0; r < h; r++, buffer += rowsize)
        {
            fmt_convert_row((*bm)[r], g, w, buffer);
        }
    }
}

static void fmt_dither(GPixmap *pm, int x, int y)
{
    if (GlobalRenderFormat::instance().ditherbits < 8)
    {
        return;
    }
    else if (GlobalRenderFormat::instance().ditherbits < 15)
    {
        pm->ordered_666_dither(x, y);
    }
    else if (GlobalRenderFormat::instance().ditherbits < 24)
    {
        pm->ordered_32k_dither(x, y);
    }
}

static int renderDjVuPage(GP<DjVuImage> image,
                          const DjVuRenderMode mode,
                          const QRect & page_rect,
                          const QRect & render_rect,
                          unsigned long rowsize,
                          char *image_buffer )
{
    try
    {
        GP<GPixmap> pm;
        GP<GBitmap> bm;
        GRect prect, rrect;
        qRect2GRect(page_rect, prect);
        qRect2GRect(render_rect, rrect);

        if (GlobalRenderFormat::instance().ytoptobottom)
        {
            prect.ymin = render_rect.top() + render_rect.height();
            prect.ymax = prect.ymin + page_rect.height();
            rrect.ymin = page_rect.top() + page_rect.height();
            rrect.ymax = rrect.ymin + render_rect.height();
        }

        if (image) 
        {
            switch (mode)
            {
            case DDJVU_RENDER_COLOR:
                pm = image->get_pixmap(rrect, prect, GlobalRenderFormat::instance().gamma);
                if (pm == 0)
                {
                    bm = image->get_bitmap(rrect, prect);
                }
                break;
            case DDJVU_RENDER_BLACK:
                bm = image->get_bitmap(rrect, prect);
                if (bm == 0)
                {
                    pm = image->get_pixmap(rrect, prect, GlobalRenderFormat::instance().gamma);
                }
                break;
            case DDJVU_RENDER_MASKONLY:
                bm = image->get_bitmap(rrect, prect);
                break;
            case DDJVU_RENDER_COLORONLY:
                pm = image->get_pixmap(rrect, prect, GlobalRenderFormat::instance().gamma);
                break;
            case DDJVU_RENDER_BACKGROUND:
                pm = image->get_bg_pixmap(rrect, prect, GlobalRenderFormat::instance().gamma);
                break;
            case DDJVU_RENDER_FOREGROUND:
                pm = image->get_fg_pixmap(rrect, prect, GlobalRenderFormat::instance().gamma);
                if (pm == 0)
                {
                    bm = image->get_bitmap(rrect, prect);
                }
                break;
            }
        }
        if (pm != 0)
        {
            int dx = rrect.xmin - prect.xmin;
            int dy = rrect.ymin - prect.xmin;
            fmt_dither(pm, dx, dy);
            fmt_convert(pm, image_buffer, rowsize);
            return 2;
        }
        else if (bm != 0)
        {
            fmt_convert(bm, image_buffer, rowsize);
            return 1;
        }
    }
    catch (GException&)
    {
    }
    catch (...)
    {
    }
    return 0;
}

static bool getContentFromPage(const QImage & page,
                               const int width,
                               const int height,
                               QRect & content)
{
    static const int LINE_STEP        = 1;
    static const int SHRINK_STEP      = 1;
    static const double STOP_RANGE    = 0.3f;
    static const int BACKGROUND_COLOR = 0xffffffff;

    // top left
    int x1 = 0;
    int y1 = 0;
    // bottom right
    int x2 = width - 1;
    int y2 = height - 1;

    int left_edge = static_cast<int>(STOP_RANGE * x2);
    int right_edge = static_cast<int>((1.0f - STOP_RANGE) * x2);
    int top_edge = static_cast<int>(STOP_RANGE * y2);
    int bottom_edge = static_cast<int>((1.0f - STOP_RANGE) * y2);

    // current pixel
    QRgb cur_pix;
    bool stop[4] = {false, false, false, false};
    while (!stop[0] || !stop[1] || !stop[2] || !stop[3])
    {
        // check top line
        int x_cur = x1;
        while (x_cur < x2 && !stop[0])
        {
            cur_pix = page.pixel(x_cur, y1);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[0] = true;
                break;
            }
            x_cur += LINE_STEP;
        }

        // check bottom line
        x_cur = x1;
        while (x_cur < x2 && !stop[1])
        {
            cur_pix = page.pixel(x_cur, y2);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[1] = true;
                break;
            }
            x_cur += LINE_STEP;
        }

        // check left line
        int y_cur = y1;
        while (y_cur < y2 && !stop[2])
        {
            cur_pix = page.pixel(x1, y_cur);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[2] = true;
                break;
            }
            y_cur += LINE_STEP;
        }

        // check right line
        y_cur = y1;
        while (y_cur < y2 && !stop[3])
        {
            cur_pix = page.pixel(x2, y_cur);
            if (cur_pix != BACKGROUND_COLOR)
            {
                stop[3] = true;
                break;
            }
            y_cur += LINE_STEP;
        }

        // shrink the rectangle
        if (!stop[2])
        {
            if (x1 >= left_edge)
            {
                stop[2] = true;
            }
            else
            {
                x1 += SHRINK_STEP;
            }
        }

        if (!stop[3])
        {
            if (x2 <= right_edge)
            {
                stop[3] = true;
            }
            else
            {
                x2 -= SHRINK_STEP;
            }
        }

        if (!stop[0])
        {
            if (y1 >= top_edge)
            {
                stop[0] = true;
            }
            else
            {
                y1 += SHRINK_STEP;
            }
        }

        if (!stop[1])
        {
            if (y2 <= bottom_edge)
            {
                stop[1] = true;
            }
            else
            {
                y2 -= SHRINK_STEP;
            }
        }

    }

    if (stop[0] && stop[1] && stop[2] && stop[3])
    {
        content.setTopLeft(QPoint(x1, y1));
        content.setBottomRight(QPoint(x2, y2));
        return true;
    }
    return false;
}

static void expandContentArea(const int page_width,
                              const int page_height,
                              QRect & content_area)
{
    static const int EXPAND_STEP = 1;
    // expand the content area to avoid content covering
    int inc_right = 0;
    int inc_bottom = 0;

    // expand left
    if (content_area.left() > EXPAND_STEP)
    {
        content_area.setLeft(content_area.left() - EXPAND_STEP);
        inc_right = EXPAND_STEP;
    }
    else
    {
        inc_right = content_area.left();
        content_area.setLeft(0);
    }

    // expand top
    if (content_area.top() > EXPAND_STEP)
    {
        content_area.setTop(content_area.top() - EXPAND_STEP);
        inc_bottom = EXPAND_STEP;
    }
    else
    {
        inc_bottom = content_area.top();
        content_area.setTop(0);
    }

    // expand right
    content_area.setRight(content_area.right() + inc_right + 1);
    if (content_area.right() > page_width)
    {
        content_area.setRight(page_width);
    }

    // expand bottom
    content_area.setBottom(content_area.bottom() + inc_bottom + 1);
    if (content_area.bottom() > page_height)
    {
        content_area.setBottom(page_height);
    }
}

unsigned int tryCalcImageLength(const int width, const int height, QImage::Format f)
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

inline int getTotalRotate(GP<DjVuImage> image, int rotate)
{
    GP<DjVuInfo> info = image->get_info();
    return (rotate + (info != 0 ? info->orientation : 0)) % 4;
}

// DjVuPageInfo ---------------------------------------------------------------

DjVuPageInfo::DjVuPageInfo()
: decoded(false)
, has_text(false)
, anno_decoded(false)
, text_decoded(false)
, initial_rotation_degree(0)
, dpi(160)
, ant(0)
, text(0)
{
}

DjVuPageInfo::~DjVuPageInfo()
{
}

void DjVuPageInfo::update(GP<DjVuImage> image)
{
    if (image == 0)
    {
        return;
    }

    if (!decoded)
    {
        initial_rotation_degree = getTotalRotate(image, 0);
        page_size = QSize(image->get_width(), image->get_height());
        if (initial_rotation_degree % 2 != 0)
        {
            page_size.transpose();
        }

        dpi = image->get_dpi();
        if (page_size.width() <= 0 || page_size.height() <= 0)
        {
            page_size = QSize(100, 100);
            dpi = 160;
        }

        has_text = !!(image->get_djvu_file()->text != 0);
        decoded = true;
    }

    try
    {
        if (has_text && !text_decoded)
        {
            decodeText(image->get_text());
        }
    }
    catch (GException&)
    {
    }

    try
    {
        if (!anno_decoded)
        {
            decodeAnno(image->get_anno());
        }
    }
    catch (GException&)
    {
    }
}

void DjVuPageInfo::update(const DjVuPageInfo & info)
{
    if (!decoded && info.decoded)
    {
        initial_rotation_degree = info.initial_rotation_degree;
        page_size = info.page_size;
        dpi = info.dpi;
        has_text = info.has_text;
        decoded = true;
    }

    if (!text_decoded && info.text_decoded)
    {
        text = info.text;
        text_decoded = true;
    }

    if (!anno_decoded && info.anno_decoded)
    {
        ant = info.ant;
        //anno = info.anno;    // TODO. Implement loading annotation
        anno_decoded = true;
    }
}

void DjVuPageInfo::decodeAnno(GP<ByteStream> anno_stream)
{
}

void DjVuPageInfo::decodeText(GP<ByteStream> text_stream)
{
}

// DjVuPage -------------------------------------------------------------------
DjVuPage::DjVuPage(int page_num)
: djvu_image_(0)
, page_num_(page_num)
{
    initialColorTable();
}

DjVuPage::~DjVuPage()
{
}

bool DjVuPage::djvuImageDecoded(DjVuSource * source)
{
    GP<DjVuFile> file = source->getDjVuDoc()->get_djvu_file(page_num_);
    return djvu_image_ != 0 && file->is_decode_ok();
}

void DjVuPage::clearImage()
{
    if (!image_.isNull())
    {
        image_ = QImage();
    }
}

int DjVuPage::imageLength()
{
    if (!image_.isNull())
    {
        return image_.numBytes();
    }
    return 0;
}

bool DjVuPage::render(DjVuSource * source, const RenderSetting & render_setting)
{
    if (!image_.isNull() && render_setting_ == render_setting)
    {
        // update render setting whenever
        render_setting_ = render_setting;
        return true;
    }

    if (!djvuImageDecoded(source))
    {
        return false;
    }

    if (render_setting_ != render_setting &&
        !source->pageManager()->makeEnoughMemory(tryCalcImageLength(
                                                    render_setting.contentArea().width(),
                                                    render_setting.contentArea().height(),
                                                    QImage::Format_Indexed8),
                                                    page_num_))
    {
        // clear memory fails
        return false;
    }

    QRect render_rect = render_setting.contentArea();
    QImage image(render_setting.contentArea().size(), QImage::Format_Indexed8);
    image.setColorTable(COLOR_TABLE);
    render_setting_ = render_setting;

    int ret = renderDjVuPage(djvu_image_, DDJVU_RENDER_BLACK, render_setting.contentArea(),
                             render_rect, image.bytesPerLine(), (char*)image.bits());
    if (ret > 0)
    {
        image_ = image;
        return true;
    }

    if (!image_.isNull())
    {
        image_ = QImage();
    }
    return false;
}

QRect DjVuPage::getContentArea(DjVuSource * source)
{
    if (content_area_.isValid() || !djvuImageDecoded(source))
    {
        return content_area_;
    }

    // intialize the content area
    DjVuPageInfo info = source->getPageInfo(page_num_);
    content_area_.setTopLeft(QPoint(0, 0));
    content_area_.setSize(info.page_size);

    static const ZoomFactor MAX_SAMPLE_SIZE = 200.0f;
    ZoomFactor zoom = min(MAX_SAMPLE_SIZE /
                          static_cast<ZoomFactor>(info.page_size.width()),
                          MAX_SAMPLE_SIZE /
                          static_cast<ZoomFactor>(info.page_size.height()));

    int width = static_cast<int>(zoom * static_cast<ZoomFactor>(info.page_size.width()));
    int height = static_cast<int>(zoom * static_cast<ZoomFactor>(info.page_size.height()));

    QRect page_rect(QPoint(0, 0), QSize(width, height));
    QRect render_rect(page_rect);
    QImage image(QSize(width, height), QImage::Format_Indexed8);
    image.setColorTable(COLOR_TABLE);
    int ret = renderDjVuPage(djvu_image_, DDJVU_RENDER_BLACK, page_rect,
                             render_rect, image.bytesPerLine(), (char*)image.bits());
    if (ret > 0 &&
        getContentFromPage(image,
                           width,
                           height,
                           content_area_))
    {
        expandContentArea(width, height, content_area_);
        content_area_.setTopLeft(QPoint(
            static_cast<ZoomFactor>(content_area_.left()) / zoom,
            static_cast<ZoomFactor>(content_area_.top()) / zoom));
        content_area_.setBottomRight(QPoint(
            static_cast<ZoomFactor>(content_area_.right()) / zoom,
            static_cast<ZoomFactor>(content_area_.bottom()) / zoom));
    }
    return content_area_;
}

int DjVuPage::getTextPosition(DjVuSource * source, const QPoint & pos_in_page, bool return_block_start)
{
    if (info_.has_text && !info_.text_decoded)
    {
        info_.update(source->readPageInfo(page_num_, true));
    }

    if (info_.text == 0)
    {
        return 0;
    }

    int text_pos = -1;
    double best = 1e10;
    getTextPosition(info_.text->page_zone, pos_in_page, text_pos, best, return_block_start);

    if (text_pos == -1)
    {
        text_pos = info_.text->textUTF8.length();
    }
    return text_pos;
}

void DjVuPage::getTextPosition(const DjVuTXT::Zone & zone,
                               const QPoint & pos_in_page,
                               int & text_pos,
                               double & best,
                               bool return_block_start)
{
    if (!zone.children.isempty())
    {
        for (GPosition pos = zone.children; pos; ++pos)
        {
            getTextPosition(zone.children[pos], pos_in_page, text_pos, best, return_block_start);
        }
        return;
    }

    QPoint pos_diff(0, 0);
    if (zone.rect.xmin > pos_in_page.x())
    {
        pos_diff.setX(zone.rect.xmin - pos_in_page.x());
    }
    else if (zone.rect.xmax <= pos_in_page.x())
    {
        pos_diff.setX(zone.rect.xmax - pos_in_page.x() - 1);
    }

    if (zone.rect.ymax <= pos_in_page.y())
    {
        pos_diff.setY(pos_in_page.y() - zone.rect.ymax + 1);
    }
    else if (zone.rect.ymin > pos_in_page.y())
    {
        pos_diff.setY(pos_in_page.y() - zone.rect.ymin);
    }

    double distance = pow(pow(pos_diff.x(), 2.0) + pow(pos_diff.y(), 2.0), 0.5);
    if (distance < best)
    {
        best = distance;
        if (!return_block_start && (pos_diff.x() < 0 || pos_diff.x() == 0 && pos_diff.y() < 0))
        {
            const DjVuTXT::Zone* current_zone = &zone;
            const DjVuTXT::Zone* parent = current_zone->get_parent();
            while (parent != 0 && &parent->children[parent->children.lastpos()] == current_zone)
            {
                current_zone = parent;
                parent = parent->get_parent();
            }

            text_pos = current_zone->text_start + current_zone->text_length;
        }
        else
        {
            text_pos = zone.text_start;
        }
    }
}

}
