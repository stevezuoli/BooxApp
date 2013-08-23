#ifndef ONYX_CATALOG_INTERFACE_H_
#define ONYX_CATALOG_INTERFACE_H_

#include <QtCore/QtCore>
#include "download_manager.h"
#include "content.h"

class CatalogInterface : public QObject
{
public:
    virtual bool get(DownloadManager & network, OContent & root) = 0;

};

#endif
