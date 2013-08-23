#ifndef DJVU_SOURCE_H_
#define DJVU_SOURCE_H_

#include "djvu_page.h"
#include "djvu_page_manager.h"


using namespace vbf;

namespace djvu_reader
{

class DjVuSource : public QObject
{
    Q_OBJECT
public:
    DjVuSource();
    virtual ~DjVuSource();

    inline vbf::RenderPolicy* renderPolicy() { return page_manager_.renderPolicy(); }
    inline DjvuPageManager* pageManager() { return &page_manager_; }

    bool          open(const QString & file_path);
    DjVuPagePtr   getPage(int page_num);
    int           getPageCount();

    int           getPageFromId(const GUTF8String& strPageId) const;

    GP<DjVmNav>       getContents();
    GP<DjVuDocument>  getDjVuDoc();

    GP<DjVuImage> getPageDjvuImage(int page_num, bool sync = true);
    DjVuPageInfo  getPageInfo(int page_num, bool need_text = false, bool need_anno = false);

    const QString&    fileName() const;

    void render(int page_num, const RenderSetting & render_setting);
    void prerender(int page_num, const RenderSetting & render_setting);
    void requirePageContentArea(int page_num);

Q_SIGNALS:
    void pageRenderReady(DjVuPagePtr page);
    void pageContentAreaReady(DjVuPagePtr page);

private:
    DjVuPageInfo readPageInfo(int page_num, bool need_text = false, bool need_anno = false);
    void readAnnotations(GP<ByteStream> inc_stream, set<GUTF8String> & processed, GP<ByteStream> anno_stream);

    void notifyPageRenderReady(DjVuPagePtr page);
    void notifyPageContentAreaReady(DjVuPagePtr page);

private:
    GP<DjVuDocument> djvu_doc_;
    QString          file_name_;
    int              page_count_;
    bool             has_text_;
    DjvuPageManager  page_manager_;

    TasksHandler     tasks_handler_;

    friend class DjVuRenderTask;
    friend class DjVuGetContentTask;
    friend class DjVuPage;
};

};
#endif // DJVU_SOURCE_H_
