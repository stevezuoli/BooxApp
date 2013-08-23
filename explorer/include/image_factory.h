#ifndef IMAGE_FACTORY_H_
#define IMAGE_FACTORY_H_

#include "onyx/base/base.h"
#include "onyx/cms/content_thumbnail.h"
#include "node.h"

using namespace cms;

namespace explorer
{

using namespace model;

namespace view
{

/// Manage all content images for explorer.
class ImageFactory
{
public:
    static ImageFactory & instance()
    {
        static ImageFactory instance_;
        return instance_;
    }

    ~ImageFactory(void);

public:
    const QImage & image(const model::Node *, const cms::ThumbnailType);

private:
    ImageFactory(void);

    void initializeExtensionNames();
    void initializeLocations();
    void clear();

    const QImage & imageFromType(const int,
                                 const ThumbnailType);
    const QImage & imageFromName(const QString & name,
                                 const ThumbnailType);
    const QImage & imageFromExtension(const model::Node *,
                                      const QString &,
                                      const ThumbnailType);

private:
    typedef unordered_map<int, QString> LocationTable;
    typedef LocationTable::iterator LocationTableIter;

    typedef QHash<QString, int> Ext2TypeTable;
    typedef Ext2TypeTable::iterator Ext2TypeTableIter;

    typedef QImage * ImagePtr;
    typedef unordered_map<int, ImagePtr> ImageTable;
    typedef ImageTable::iterator ImageTableIter;

    typedef QHash< QString , ImagePtr> WebImageTable; 

    Ext2TypeTable extension_table_;

    LocationTable small_images_location_;
    LocationTable middle_images_location_;
    LocationTable big_images_location_;

    ImageTable small_images_;
    ImageTable middle_images_;
    ImageTable big_images_;

    WebImageTable web_small_images_;
    WebImageTable web_middle_images_;
    WebImageTable web_big_images_;
private:
    void initializeLocationTable(const QString & base_path,
                                 LocationTable &table);

    //int idForName(const QString &name);

    const QImage & makeSureImageExist(const int id,
                                      ImageTable & Images,
                                      LocationTable & locations);

    void clearImages(ImageTable & Images);

    NO_COPY_AND_ASSIGN(ImageFactory);
};

}  // end of view

}  // end of explorer
#endif
