#ifndef ONYX_OPDS_IMPL_H_
#define ONYX_OPDS_IMPL_H_

#include "catalog_interface.h"

class OPDSImpl : public CatalogInterface
{
    Q_OBJECT

public:
    virtual bool get(DownloadManager & network, OContent & root);

};

#endif
