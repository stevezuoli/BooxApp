#include "djvu_source.h"
#include "djvu_task.h"
#include "djvu_render_policy.h"

namespace djvu_reader
{

DjVuSource::DjVuSource()
    : djvu_doc_(0)
    , page_count_(0)
    , has_text_(false)
{
}

DjVuSource::~DjVuSource()
{
}

bool DjVuSource::open(const QString & file_path)
{
    try
    {
        GURL url = GURL::Filename::UTF8(GUTF8String(file_path.toUtf8().constData()));
        djvu_doc_ = DjVuDocument::create(url);
        djvu_doc_->wait_get_pages_num();
    }
    catch (GException&)
    {
        return false;
    }

    page_count_ = djvu_doc_->get_pages_num();
    return page_count_ != 0;
}

DjVuPagePtr DjVuSource::getPage(int page_num)
{
    DjVuPagePtr page = page_manager_.getPage(page_num);
    return page;
}

int DjVuSource::getPageCount()
{
    return page_count_;
}

int DjVuSource::getPageFromId(const GUTF8String& strPageId) const
{
    // TODO. Implement Me
    return 0;
}

GP<DjVmNav> DjVuSource::getContents()
{
    return djvu_doc_->get_djvm_nav();
}

GP<DjVuDocument> DjVuSource::getDjVuDoc()
{
    return djvu_doc_;
}

const QString& DjVuSource::fileName() const
{
    return file_name_;
}

GP<DjVuImage> DjVuSource::getPageDjvuImage(int page_num, bool sync)
{
    DjVuPagePtr page = getPage(page_num);
    GP<DjVuImage> image = page->getDjvuImage();
    GP<DjVuFile> file = djvu_doc_->get_djvu_file(page_num);
    assert(file != 0);
    if (image != 0)
    {
        if (!file->is_decode_ok())
        {
            file->resume_decode(sync);
        }
    }
    else
    {
        try
        {
            image = DjVuImage::create(file);
            file->resume_decode(sync);
        }
        catch (GException&)
        {
        }
        catch (...)
        {
        }
        page->djvu_image_ = image;
    }
    return image;
}

DjVuPageInfo DjVuSource::getPageInfo(int page_num, bool need_text, bool need_anno)
{
    assert(page_num >= 0 && page_num < page_count_);
    DjVuPagePtr page = page_manager_.getPage(page_num);
    DjVuPageInfo page_info = page->getPageInfo();
    if (page_info.decoded &&
        (page_info.text_decoded || !need_text) &&
        (page_info.anno_decoded || !need_anno))
    {
        return page_info;
    }

    DjVuPageInfo info = readPageInfo(page_num, need_text, need_anno);
    page->info_.update(info);
    if (info.has_text)
    {
        has_text_ = true;
    }
    return info;
}

DjVuPageInfo DjVuSource::readPageInfo(int page_num, bool need_text, bool need_anno)
{
    assert(page_num >= 0 && page_num < page_count_);
    DjVuPageInfo page_info;
    page_info.page_size.setWidth(100);
    page_info.page_size.setHeight(100);
    page_info.dpi = 160;
    page_info.decoded = true;

    GP<ByteStream> anno_stream;
    if (need_anno)
    {
        anno_stream = ByteStream::create();
    }

    GP<ByteStream> text_stream;
    if (need_text)
    {
        text_stream = ByteStream::create();
    }

    try
    {
        // Get raw data from the document and decode only requested chunks
        // DjVuFile is not used to ensure that we do not wait for a lock
        // to be released and thus do not block the UI thread
        GURL url = djvu_doc_->page_to_url(page_num);
        GP<DataPool> pool = djvu_doc_->request_data(0, url);
        GP<ByteStream> stream = pool->get_stream();
        GP<IFFByteStream> iff(IFFByteStream::create(stream));

        // Check file format
        GUTF8String chkid;
        if (!iff->get_chunk(chkid) ||
            (chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
             chkid != "FORM:PM44" && chkid != "FORM:BM44"))
        {
            return page_info;
        }

        bool has_IW44 = false;

        // Find chunk with page info
        while (iff->get_chunk(chkid) != 0)
        {
            GP<ByteStream> chunk_stream = iff->get_bytestream();

            if (chkid == "INFO")
            {
                // Get page dimensions and resolution from info chunk
                GP<DjVuInfo> info = DjVuInfo::create();
                info->decode(*chunk_stream);

                // Check data for consistency
                page_info.page_size.setWidth(max(info->width, 0));
                page_info.page_size.setHeight(max(info->height, 0));
                page_info.initial_rotation_degree = info->orientation;
                page_info.dpi = max(info->dpi, 0);

                if ((info->orientation & 1) != 0)
                {
                    page_info.page_size.transpose();
                }
            }
            else if (!has_IW44 && (chkid == "PM44" || chkid == "BM44"))
            {
                has_IW44 = true;

                // Get image dimensions and resolution from bitmap chunk
                UINT serial = chunk_stream->read8();
                UINT slices = chunk_stream->read8();
                UINT major = chunk_stream->read8();
                UINT minor = chunk_stream->read8();

                UINT xhi = chunk_stream->read8();
                UINT xlo = chunk_stream->read8();
                UINT yhi = chunk_stream->read8();
                UINT ylo = chunk_stream->read8();

                page_info.page_size.setWidth((xhi << 8) | xlo);
                page_info.page_size.setHeight((yhi << 8) | ylo);
                page_info.dpi = 160;
            }
            else if (chkid == "TXTa" || chkid == "TXTz")
            {
                page_info.has_text = true;

                if (need_text)
                {
                    const GP<IFFByteStream> iffout = IFFByteStream::create(text_stream);
                    iffout->put_chunk(chkid);
                    iffout->copy(*chunk_stream);
                    iffout->close_chunk();
                }
            }
            else if (need_anno && chkid == "FORM:ANNO")
            {
                anno_stream->copy(*chunk_stream);
            }
            else if (need_anno && (chkid == "ANTa" || chkid == "ANTz"))
            {
                const GP<IFFByteStream> iffout = IFFByteStream::create(anno_stream);
                iffout->put_chunk(chkid);
                iffout->copy(*chunk_stream);
                iffout->close_chunk();
            }
            else if (need_anno && chkid == "INCL")
            {
                set<GUTF8String> processed;
                readAnnotations(chunk_stream, processed, anno_stream);
            }

            iff->seek_close_chunk();
        }

        if (need_text && text_stream->tell())
        {
            page_info.decodeText(text_stream);
        }

        if (need_anno && anno_stream->tell())
        {
            page_info.decodeAnno(anno_stream);
        }
    }
    catch (GException&)
    {
    }
    catch (...)
    {
    }

    return page_info;
}

void DjVuSource::readAnnotations(GP<ByteStream> inc_stream, set<GUTF8String> & processed, GP<ByteStream> anno_stream)
{
    // Look for shared annotations
    GUTF8String str_include;
    char buf[1024];
    int length = 0;
    while ((length = inc_stream->read(buf, 1024)))
    {
        str_include += GUTF8String(buf, length);
    }

    // Eat '\n' in the beginning and at the end
    while (str_include.length() > 0 && str_include[0] == '\n')
    {
        str_include = str_include.substr(1, static_cast<unsigned int>(-1));
    }

    while (str_include.length() > 0 && str_include[static_cast<int>(str_include.length()) - 1] == '\n')
    {
        str_include.setat(str_include.length() - 1, 0);
    }

    if (str_include.length() > 0 && processed.find(str_include) == processed.end())
    {
        processed.insert(str_include);

        GURL url_include = djvu_doc_->id_to_url(str_include);
        GP<DataPool> pool = djvu_doc_->request_data(0, url_include);
        GP<ByteStream> stream = pool->get_stream();
        GP<IFFByteStream> iff(IFFByteStream::create(stream));

        // Check file format
        GUTF8String chkid;
        if (!iff->get_chunk(chkid) ||
            (chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
            chkid != "FORM:PM44" && chkid != "FORM:BM44"))
        {
            return;
        }

        // Find chunk with page info
        while (iff->get_chunk(chkid) != 0)
        {
            GP<ByteStream> chunk_stream = iff->get_bytestream();

            if (chkid == "INCL")
            {
                readAnnotations(inc_stream, processed, anno_stream);
            }
            else if (chkid == "FORM:ANNO")
            {
                anno_stream->copy(*chunk_stream);
            }
            else if (chkid == "ANTa" || chkid == "ANTz")
            {
                const GP<IFFByteStream> iffout = IFFByteStream::create(anno_stream);
                iffout->put_chunk(chkid);
                iffout->copy(*chunk_stream);
                iffout->close_chunk();
            }

            iff->seek_close_chunk();
        }
    }
}

void DjVuSource::requirePageContentArea(int page_num)
{
    assert(page_num >= 0 && page_num < page_count_);

    DjVuPagePtr page = page_manager_.getPage(page_num);
    if (page->content_area_.isValid())
    {
        notifyPageContentAreaReady(page);
        return;
    }

    DjVuGetContentTask * task = new DjVuGetContentTask(this, page_num);
    tasks_handler_.addTask(task, false, false, false);
}

void DjVuSource::render(int page_num, const RenderSetting & render_setting)
{
    assert(page_num >= 0 && page_num < page_count_);

    DjVuPagePtr page = page_manager_.getPage(page_num);
    if (page->image() != 0 && page->image()->isNull() && page->renderSetting() == render_setting)
    {
        notifyPageRenderReady(page);
        return;
    }

    DjVuRenderTask * task = new DjVuRenderTask(this, page_num, render_setting);
    tasks_handler_.addTask(task, false);
}

void DjVuSource::prerender(int page_num, const RenderSetting & render_setting)
{
    DjVuRenderTask * task = new DjVuRenderTask(this, page_num, render_setting, true);
    tasks_handler_.addTask(task, true);
}

void DjVuSource::notifyPageRenderReady(DjVuPagePtr page)
{
    emit pageRenderReady(page);
}

void DjVuSource::notifyPageContentAreaReady(DjVuPagePtr page)
{
    emit pageContentAreaReady(page);
}

}
