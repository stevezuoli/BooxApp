#include "notes_doc_manager.h"
#include "notes_view.h"
#include "image_model.h"

namespace image
{

NotesDocumentManager::NotesDocumentManager()
    : default_image_key_(sys::SystemConfig::notesTemplate(0))
    , need_select_background_(false)
    , content_manager_(0)
{
    if (!isBackgroundExisting(default_image_key_))
    {
        default_image_key_ = emptyBackground();
    }
}

NotesDocumentManager::~NotesDocumentManager()
{
}

void NotesDocumentManager::attachContentManager(cms::ContentManager & cms)
{
    content_manager_ = &cms;
    if (vbf::openDatabase(notes_doc_path_, *content_manager_))
    {
        vbf::loadDocumentOptions(*content_manager_, notes_doc_path_, conf_);
    }
}

bool NotesDocumentManager::load(const QString & name, SketchProxy *sketch_proxy)
{
    // set notes document name
    notes_doc_path_ = name;

    // get the default image key
    if (!sketch_proxy->open(notes_doc_path_))
    {
        return false;
    }

    // get the key of first page
    sketch::PageKey first_page = sketch_proxy->getFirstPage(notes_doc_path_);
    if (first_page.isEmpty())
    {
        need_select_background_ = true;
    }
    else
    {
        QString first_background = sketch_proxy->getBackgroundImage(notes_doc_path_, first_page);
        if (isBackgroundExisting(first_background))
        {
            default_image_key_ = first_background;
        }
        else
        {
            if (!isBackgroundExisting(default_image_key_))
            {
                default_image_key_ = emptyBackground();
            }
            need_select_background_ = true;
        }
    }
    return true;
}

QString NotesDocumentManager::getExportedImagePath(const QString & image_name, int idx)
{
    int dot_pos = image_name.lastIndexOf('.');
    QString name = image_name.left(dot_pos);
    QString post_fix = image_name.right(image_name.size() - dot_pos);
    QString exported_name("%1_%2%3");
    exported_name = exported_name.arg(name).arg(idx).arg(post_fix);

    QString exported_path;
    QDir exported_dir;
    if (sys::SystemConfig::notesExportDirectory(notes_doc_path_, exported_dir))
    {
        exported_path = exported_dir.absoluteFilePath(exported_name);
    }
    return exported_path;
}

bool NotesDocumentManager::isBackgroundExisting(const QString & background)
{
    QFileInfo file_info(background);
    if (file_info.exists())
    {
        return ImageModel::isImage(background);
    }
    return false;
}

QString NotesDocumentManager::emptyBackground()
{
    return EMPTY_BACKGROUND;
}

shared_ptr<ImageItem> NotesDocumentManager::emptyBackgroundImage()
{
    static shared_ptr<ImageItem> empty_image;
    if (empty_image == 0)
    {
        empty_image.reset(new ImageItem(EMPTY_BACKGROUND));
    }
    return empty_image;
}

bool NotesDocumentManager::saveNoteIndex(SketchProxy *sketch_proxy)
{
    if (content_manager_ == 0) return false;

    cms::NoteInfo info;
    info.mutable_name() = notes_doc_path_;

    // get thumbnail
    sketch::PageKey first_page = sketch_proxy->getFirstPage(notes_doc_path_);
    if (!first_page.isEmpty())
    {
        QString background_path = sketch_proxy->getBackgroundImage(notes_doc_path_, first_page);
        if (!background_path.isEmpty())
        {
            scoped_ptr<QImage> background;
            if (!isBackgroundExisting(background_path))
            {
                background.reset(new QImage(EMPTY_BACKGROUND_WIDTH, EMPTY_BACKGROUND_HEIGHT, QImage::Format_ARGB32));
                QColor white(255, 255, 255);
                background->fill(white.rgba());
            }
            else
            {
                background.reset(new QImage(background_path));
            }
            QSize background_size = background->size();
            QSize thumbnail_size = cms::thumbnailSize();
            ZoomFactor zoom = static_cast<ZoomFactor>(thumbnail_size.width()) /
                              static_cast<ZoomFactor>(background_size.width());

            *background = background->scaled(thumbnail_size);
            if (background->depth() < 24)
            {
                *background = background->convertToFormat(QImage::Format_ARGB32);
            }

            if (sketch_proxy->loadPage(notes_doc_path_, first_page, QString()))
            {
                QPainter painter(background.get());
                sketch_proxy->deactivateAll();
                sketch_proxy->activatePage(notes_doc_path_, first_page);

                sketch_proxy->setZoom( zoom * ZOOM_ACTUAL);
                sketch_proxy->setWidgetOrient(getSystemRotateDegree());
                QRect page_area(QPoint(0, 0), background->size());
                sketch_proxy->updatePageDisplayRegion(notes_doc_path_, first_page, page_area);
                sketch_proxy->paintPage(notes_doc_path_, first_page, painter);
            }
            info.mutable_thumbnail() = *background;
        }
    }
    return content_manager_->addNoteIndex(info);
}

bool NotesDocumentManager::exportImages(SketchProxy *sketch_proxy)
{
    sketch::PageKey last_page = sketch_proxy->getLastPage(notes_doc_path_);
    if (last_page.isEmpty())
    {
        return false;
    }

    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    sys::SysStatus::instance().setSystemBusy(true);
    int last_page_num = 0;
    last_page_num = last_page.toInt();

    // export each page by background id
    for (int i = 0; i <= last_page_num; ++i)
    {
        // get background image
        sketch::PageKey current_page_key;
        current_page_key.setNum(i);
        QString background_path = sketch_proxy->getBackgroundImage(notes_doc_path_, current_page_key);
        if (!background_path.isEmpty())
        {
            scoped_ptr<QImage> background;
            if (!isBackgroundExisting(background_path))
            {
                background.reset(new QImage(EMPTY_BACKGROUND_WIDTH, EMPTY_BACKGROUND_HEIGHT, QImage::Format_ARGB32));
                QColor white(255, 255, 255);
                background->fill(white.rgba());
            }
            else
            {
                background.reset(new QImage(background_path));
            }

            if (background->depth() < 24)
            {
                *background = background->convertToFormat(QImage::Format_ARGB32);
            }

            if (sketch_proxy->loadPage(notes_doc_path_, current_page_key, QString()) &&
                !sketch_proxy->isEmptyPage(notes_doc_path_, current_page_key))
            {
                QPainter painter(background.get());
                sketch_proxy->deactivateAll();
                sketch_proxy->activatePage(notes_doc_path_, current_page_key);
                sketch_proxy->setZoom( 1.0 * ZOOM_ACTUAL);
                sketch_proxy->setWidgetOrient(getSystemRotateDegree());

                QRect page_area(QPoint(0, 0), background->size());
                sketch_proxy->updatePageDisplayRegion(notes_doc_path_, current_page_key, page_area);
                sketch_proxy->paintPage(notes_doc_path_, current_page_key, painter);
            }

            QFileInfo background_info(background_path);
            background->save(getExportedImagePath(background_info.fileName(), i));
        }
    }

    // export the sketch DB
    sketch_proxy->exportDatabase(notes_doc_path_);
    sys::SysStatus::instance().setSystemBusy(false);
    return true;
}

bool NotesDocumentManager::saveConf()
{
    if (content_manager_ == 0) return false;
    return vbf::saveDocumentOptions(*content_manager_, notes_doc_path_, conf_);
}

void NotesDocumentManager::setDefaultBackground(const ImageKey & key)
{
    default_image_key_ = key;
    if (!isBackgroundExisting(default_image_key_))
    {
        default_image_key_ = emptyBackground();
    }
}

ImageKey NotesDocumentManager::defaultBackground()
{
    return default_image_key_;
}

}

