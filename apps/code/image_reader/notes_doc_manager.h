#ifndef NOTES_DOC_MANAGER_H_
#define NOTES_DOC_MANAGER_H_

#include "image_utils.h"

using namespace sketch;
namespace image
{

class ImageItem;
class NotesDocumentManager : public QObject
{
    Q_OBJECT
public:
    NotesDocumentManager();
    virtual ~NotesDocumentManager();

    inline QString notesDocumentPath() { return notes_doc_path_; }
    inline vbf::Configuration & conf() { return conf_; }
    inline bool needSelectBackground() { return need_select_background_; }

    void attachContentManager(cms::ContentManager & cms);
    void setDefaultBackground(const ImageKey & key);
    ImageKey defaultBackground();

    bool load(const QString & name, SketchProxy *sketch_proxy);
    bool exportImages(SketchProxy *sketch_proxy);
    bool saveNoteIndex(SketchProxy *sketch_proxy);
    bool saveConf();

    bool isBackgroundExisting(const QString & background);
    QString emptyBackground();
    shared_ptr<ImageItem> emptyBackgroundImage();

private:
    QString getExportedImagePath(const QString & notes_doc_path, int idx);

private:
    QString    notes_doc_path_;
    ImageKey   default_image_key_;
    bool       need_select_background_;

    vbf::Configuration conf_;
    cms::ContentManager * content_manager_;
};

};

#endif
