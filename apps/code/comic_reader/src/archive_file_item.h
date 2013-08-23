#ifndef ARCHIVE_FILE_ITEM_H_
#define ARCHIVE_FILE_ITEM_H_

#include <QtCore/QtCore>

namespace compression {

class ArchiveFileItem
{
public:
    ArchiveFileItem();
    ~ArchiveFileItem(void);


private:
    int file_index_;        ///< file index in the archive file
    QString file_name_;     ///< file name, including the internal path in
                            ///  the archive file
    QByteArray raw_data_;   ///< the raw data of a file (decompressed)
};

}   // namespace compression

#endif
