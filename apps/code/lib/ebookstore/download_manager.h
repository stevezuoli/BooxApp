#ifndef ONYX_DOWNLOAD_MANAGER_H_
#define ONYX_DOWNLOAD_MANAGER_H_

#include <QtNetwork/QtNetwork>

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    bool downloadContent();
    bool downloadFile();
};

#endif
