#ifndef DJVU_PAGE_H_
#define DJVU_PAGE_H_

#include "djvu_utils.h"

using namespace ui;
namespace djvu_reader
{

class DjVuPageInfo
{
public:
    DjVuPageInfo();
    ~DjVuPageInfo();

    void update(GP<DjVuImage> image);
    void update(const DjVuPageInfo & info);

    void decodeAnno(GP<ByteStream> anno_stream);
    void decodeText(GP<ByteStream> text_stream);

public:
    bool           decoded;
    bool           has_text;
    bool           anno_decoded;
    bool           text_decoded;
    int            initial_rotation_degree;
    int            dpi;
    GP<DjVuANT>    ant;
    GP<DjVuTXT>    text;
    QSize          page_size;
};

class DjVuSource;
class DjVuPage
{
public:
    DjVuPage(int page_num);
    ~DjVuPage();

    inline QImage * image() { return &image_; }
    inline const RenderSetting & renderSetting() const { return render_setting_; }

    inline GP<DjVuImage> getDjvuImage() const { return djvu_image_; }
    inline DjVuPageInfo  getPageInfo() { return info_; }

    inline int getPageNumber() const { return page_num_; }

    int imageLength();
    void clearImage();
    bool render(DjVuSource * source, const RenderSetting & render_setting);
    QRect getContentArea(DjVuSource * source);

    int getTextPosition(DjVuSource * source, const QPoint & pos_in_page, bool return_block_start = false);

private:
    bool djvuImageDecoded(DjVuSource * source);
    void getTextPosition(const DjVuTXT::Zone & zone,
                         const QPoint & pos_in_page,
                         int & text_pos,
                         double & best,
                         bool return_block_start);

private:
    GP<DjVuImage>               djvu_image_;
    DjVuPageInfo                info_;
    int                         page_num_;            // might become private pointer in the future
    RenderSetting               render_setting_;
    QImage                      image_;
    QRect                       content_area_;

    friend class DjVuSource;
};

typedef shared_ptr<DjVuPage> DjVuPagePtr;

};
#endif // DJVU_PAGE_H_
