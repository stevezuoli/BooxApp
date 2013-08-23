#include "image_factory.h"
#include "explorer_conf.h"
#include "file_node.h"

namespace explorer {

using namespace model;

namespace view
{

ImageFactory::ImageFactory()
    : extension_table_()
    , small_images_location_()
    , middle_images_location_()
    , big_images_location_()
    , small_images_()
    , middle_images_()
    , big_images_()
    , web_small_images_()
    , web_middle_images_()
    , web_big_images_()
{
    initializeLocations();
    initializeExtensionNames();
}

ImageFactory::~ImageFactory()
{
    clear();
}

/// Initialize extension_to_content type table. List all known content
/// type here.
void ImageFactory::initializeExtensionNames()
{
    extension_table_["txt"]       = CONTENT_TEXT;
    extension_table_["text"]      = CONTENT_TEXT;
    extension_table_["fb2"]       = CONTENT_FB2;
    extension_table_["rtf"]       = CONTENT_RTF;
    extension_table_["pdf"]       = CONTENT_PDF;
    extension_table_["htm"]       = CONTENT_HTML;
    extension_table_["html"]      = CONTENT_HTML;
    extension_table_["jpg"]       = CONTENT_JPEG;
    extension_table_["jpeg"]      = CONTENT_JPEG;
    extension_table_["png"]       = CONTENT_PNG;
    extension_table_["bmp"]       = CONTENT_BMP;
    extension_table_["gif"]       = CONTENT_GIF;
    extension_table_["chm"]       = CONTENT_CHM;
    extension_table_["pdb"]       = CONTENT_PDB;
    extension_table_["mobi"]      = CONTENT_MOBI;
    extension_table_["prc"]       = CONTENT_MOBI;
    extension_table_["epub"]      = CONTENT_EPUB;
    extension_table_["tif"]       = CONTENT_TIFF;
    extension_table_["tiff"]      = CONTENT_TIFF;
    extension_table_["mp3"]       = CONTENT_MUSIC;
    extension_table_["djvu"]      = CONTENT_DJVU;
    extension_table_["abf"]       = CONTENT_ABF;
    extension_table_["doc"]       = CONTENT_DOC;
    extension_table_["docx"]      = CONTENT_DOC;
    extension_table_["ppt"]       = CONTENT_PPT;
    extension_table_["pptx"]      = CONTENT_PPT;
    extension_table_["xls"]       = CONTENT_XLS;
    extension_table_["xlsx"]      = CONTENT_XLS;
    extension_table_["sh"]        = CONTENT_RUNNABLE;
    extension_table_["oar"]       = CONTENT_RUNNABLE;
    extension_table_["zip"]       = CONTENT_ZIP;
    extension_table_["gz"]        = CONTENT_ZIP;
    extension_table_["tar"]       = CONTENT_ZIP;
    extension_table_["rar"]       = CONTENT_ZIP;
    extension_table_["bz2"]       = CONTENT_ZIP;
    extension_table_["7z"]        = CONTENT_ZIP;
}

static QDir shareFolder()
{
#ifdef WIN32
    return QDir::home().absoluteFilePath("explorer");
#else
    QDir dir(SHARE_ROOT);
    dir.cd("explorer");
    return dir;
#endif

}

/// Initialize the location table.
void ImageFactory::initializeLocations()
{
    QString small_icons_base_path;
    QString middle_icons_base_path;
    QString big_icons_base_path;

    small_icons_base_path =
        explorerOption().value("small_images_path",
                            shareFolder().absoluteFilePath("images/small/")).toString();

    middle_icons_base_path =
        explorerOption().value("middle_images_path",
                          shareFolder().absoluteFilePath("images/middle/")).toString();
    big_icons_base_path =
        explorerOption().value("big_images_path",
                          shareFolder().absoluteFilePath("images/big/")).toString();

    // Initialize location for all Image table.
    initializeLocationTable(small_icons_base_path, small_images_location_);
    initializeLocationTable(middle_icons_base_path, middle_images_location_);
    initializeLocationTable(big_icons_base_path, big_images_location_);

    // For web sites.
}

/// Consider to support thumbnail for image file.
const QImage &
ImageFactory::image(const model::Node * node,
                    const ThumbnailType size_type)
{
    if (node->type() == NODE_TYPE_FILE)
    {
        return imageFromExtension(node, down_cast<const FileNode *>(node)->suffix(), size_type);
    }
    else if (node->type() == NODE_TYPE_WEBSITE)
    {
        return imageFromName(node->name(), size_type);
    }
    return imageFromType(node->type(), size_type);
}

void ImageFactory::clear()
{
    clearImages(small_images_);
    clearImages(middle_images_);
    clearImages(big_images_);
}

const QImage &
ImageFactory::imageFromType(const int id,
                            const ThumbnailType size_type)
{
    if (size_type == THUMBNAIL_SMALL)
    {
        return makeSureImageExist(id,
                                   small_images_,
                                   small_images_location_);
    }
    else if (size_type == THUMBNAIL_MIDDLE)
    {
        return makeSureImageExist(id,
                                   middle_images_,
                                   middle_images_location_);
    }
    else if (size_type == THUMBNAIL_LARGE)
    {
        return makeSureImageExist(id,
                                   big_images_,
                                   big_images_location_);
    }
    assert(false);
    static QImage dummy;
    return dummy;
}


const QImage &
ImageFactory::imageFromName(const QString & name,
                            const ThumbnailType size_type)
{

    WebImageTable * pImages = 0;
    QString filename;

    if (size_type == THUMBNAIL_SMALL)
    {
        filename = explorerOption().value("small_images_path",
            shareFolder().absoluteFilePath("images/small/")).toString();
        pImages  = & web_small_images_;

    }
    else if (size_type == THUMBNAIL_MIDDLE)
    {
        filename= explorerOption().value("middle_images_path",
            shareFolder().absoluteFilePath("images/middle/")).toString();

        pImages  = & web_middle_images_;
    }
    else if (size_type == THUMBNAIL_LARGE)
    {
        filename= explorerOption().value("big_images_path",
            shareFolder().absoluteFilePath("images/big/")).toString();
        pImages  = & web_big_images_;
    }
    else
    {
        assert(false);
        static QImage dummy;
        return dummy;
    }

    WebImageTable::iterator iter = pImages->find(name);
    if (iter != pImages->end())
    {
        return * iter.value();
    }

    QString tmp=filename;
    filename.append(name).append(".png") ;
    QImage *ret = new QImage;
    if (!ret->load(filename))
    {
        qWarning("load image failed name %s", qPrintable(filename) );
        // use default web site
        filename=tmp;
        filename.append("website.png");
        if (!ret->load(filename))
        {
            qWarning("load image failed name %s", qPrintable(filename) );
            *ret= imageFromType(CONTENT_UNKNOWN,size_type); 
        }
    }

    pImages->insert(name, ret);
    return *ret ;

}

const QImage &
ImageFactory::imageFromExtension(const model::Node *node,
                                 const QString & extension,
                                 const ThumbnailType size_type)
{
    ContentType content_type = CONTENT_UNKNOWN;
    Ext2TypeTableIter iter = extension_table_.find(extension.toLower());
    if (iter != extension_table_.end())
    {
        content_type = static_cast<ContentType>(iter.value());
    }

    return imageFromType(content_type, size_type);
}

void
ImageFactory::initializeLocationTable(const QString & base_path,
                                       LocationTable &table)
{
    table[NODE_TYPE_ROOT] = base_path + "root.png";
    table[NODE_TYPE_LIBRARY] = base_path + "library.png";
    table[NODE_TYPE_DICTIONARY] = base_path + "dictionary.png";
    table[NODE_TYPE_MUSICS] = base_path + "music.png";
    table[NODE_TYPE_IMAGES] = base_path + "images.png";
    table[NODE_TYPE_USB] = base_path + "usb.png";
    table[NODE_TYPE_CF] = base_path + "cf.png";
    table[NODE_TYPE_SD] = base_path + "sd.png";
    table[NODE_TYPE_LOCALE] = base_path + "locale.png";
    table[NODE_TYPE_NETWORK_PROFILE] = base_path + "network.png";
    table[NODE_TYPE_FILE] = base_path + "file.png";
    table[NODE_TYPE_DIRECTORY] = base_path + "directory.png";
    table[NODE_TYPE_RECENT_DOCS] = base_path + "recent_document.png";
    table[NODE_TYPE_NOTE_CONTAINER] = base_path + "notes.png";
    table[NODE_TYPE_NOTE] = base_path + "recent_document.png";
    table[NODE_TYPE_WRITEPAD_CONTAINER] = base_path + "write_pad.png";
    table[NODE_TYPE_NEW_WRITEPAD] = base_path + "new_write_pad.png";
    table[NODE_TYPE_WRITEPAD] = base_path + "txt.png";
    table[NODE_TYPE_NEW_NOTE] = base_path + "create_note.png";
    table[NODE_TYPE_NOTE] = base_path + "note.png";
    table[NODE_TYPE_SHORTCUTS] = base_path + "shortcut.png";
    table[NODE_TYPE_SYS_SETTINGS] = base_path + "settings.png";
    table[NODE_TYPE_DATE] = base_path + "date.png";
    table[NODE_TYPE_TIMEZONE] = base_path + "time_zone.png";
    table[NODE_TYPE_PM] = base_path + "power_management.png";
    table[NODE_TYPE_ABOUT] = base_path + "about.png";
    table[NODE_TYPE_SCREEN_CALIBRATION] = base_path + "screen_calibration.png";
    table[NODE_TYPE_WEBSITES] = base_path + "websites.png";
    table[NODE_TYPE_WEBSITE] = base_path + "website.png";
    table[NODE_TYPE_ONLINESHOPS] = base_path + "online_shops.png";
    table[NODE_TYPE_FORMAT_FLASH] = base_path + "format_flash.png";
    table[NODE_TYPE_3G_CONNECTION] = base_path + "3g.png";
    table[NODE_TYPE_SSH_SERVER] = base_path + "3g.png";
    table[NODE_TYPE_USER_MANUAL] = base_path + "user_manual.png";
    table[NODE_TYPE_WAVEFORM_SETTINGS] = base_path + "user_manual.png";
    table[NODE_TYPE_FILETYPE_SETTINGS] = base_path + "filetype_settings.png";
    table[NODE_TYPE_FEEDREADER] = base_path + "rss.png";
    table[NODE_TYPE_GAMES] = base_path + "games.png";
    table[NODE_TYPE_GAME_SUDOKU] = base_path + "suduko.png";
    table[NODE_TYPE_FONTMANAGEMENT] = base_path + "font_management.png";
    table[NODE_TYPE_APPLICATION] = base_path + "runnable_file.png";
    table[NODE_TYPE_APPLICATIONS] = base_path + "applications.png";
    table[NODE_TYPE_STARTUP] = base_path + "startup.png";
    table[NODE_TYPE_CALENDAR] = base_path + "calendar.png";
    table[NODE_TYPE_FULL_SCREEN_CLOCK] = base_path + "full_screen_clock.png";
    table[NODE_TYPE_SCREEN_UPATE_SETTING] = base_path + "screen_update_setting.png";

    table[CONTENT_BMP]  = base_path + "bmp.png";
    table[CONTENT_CHM] = base_path + "chm.png";
    table[CONTENT_EPUB] = base_path + "epub.png";
    table[CONTENT_GIF] = base_path + "gif.png";
    table[CONTENT_HTML] = base_path + "html.png";
    table[CONTENT_JPEG] = base_path + "jpg.png";
    table[CONTENT_MOBI] = base_path + "mobi.png";
    table[CONTENT_PDB] = base_path + "pdb.png";
    table[CONTENT_PDF] = base_path + "pdf.png";
    table[CONTENT_PNG]  = base_path + "png.png";
    table[CONTENT_RTF] = base_path + "rtf.png";
    table[CONTENT_TIFF] = base_path + "tiff.png";
    table[CONTENT_TEXT] = base_path + "txt.png";
    table[CONTENT_FB2] = base_path + "fb2.png";
    table[CONTENT_DJVU] = base_path + "djvu.png";
    table[CONTENT_ABF] = base_path + "abf.png";
    table[CONTENT_DOC] = base_path + "doc.png";
    table[CONTENT_XLS] = base_path + "xls.png";
    table[CONTENT_PPT] = base_path + "ppt.png";
    table[CONTENT_ZIP] = base_path + "zip.png";
    table[CONTENT_MUSIC] = base_path + "music.png";
    table[CONTENT_RUNNABLE] = base_path + "runnable_file.png";
    table[CONTENT_UNKNOWN] = base_path + "unknown_document.png";

}

/*
int ImageFactory::idForName(const QString &name)
{
    for(int i = 0; i < SITES_SIZE; ++i)
    {
        if (sites[i].compare(name, Qt::CaseInsensitive) == 0)
        {
            return i + CONTENT_END;
        }
    }
    return CONTENT_UNKNOWN;
}
*/

const QImage &
ImageFactory::makeSureImageExist(const int id,
                                   ImageTable & Images,
                                   LocationTable & locations)
{
    ImageTableIter iter = Images.find(id);
    if (iter != Images.end())
    {
        return *(iter->second);
    }

    // Load the Image.
    LocationTableIter l = locations.find(id);
    if (l == locations.end())
    {
        qWarning("id %d not available", id);
    }

    QImage *ret = new QImage;
    if (!ret->load(l->second))
    {
        qWarning("load image failed id %d", id);
    }
    Images[id] = ret;
    return *ret;
}

void ImageFactory::clearImages(ImageTable & Images)
{
    ImageTableIter begin = Images.begin();
    ImageTableIter end   = Images.end();
    for(ImageTableIter it = begin; it != end; ++it)
    {
        delete it->second;
    }
    Images.clear();
}

}  // namespace view

}  // namespace explorer
