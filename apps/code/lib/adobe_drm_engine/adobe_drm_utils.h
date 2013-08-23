#ifndef ADOBE_DRM_UTILS_H_
#define ADOBE_DRM_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

#include "onyx/base/base.h"
#include "onyx/ui/ui_global.h"
#include "onyx/sys/sys.h"
#include "onyx/sys/fat.h"
#include <QtSql/QtSql>

#include "onyx/base/base_tasks.h"
#include "onyx/base/tasks_handler.h"
#include "onyx/data/configuration.h"
#include "onyx/cms/content_manager.h"
#include "onyx/cms/cms_tags.h"

#include "network_service/access_manager.h"
#include "network_service/dm_manager.h"

namespace adobe_drm
{

enum AdobeDRMOperation
{
    FULFILL = 0,
    RETURN,
    GET_FINGERPRINT,
    UNKNOWN_OPT
};

QString getDeviceSerialNumber();
QString getVersionInfo(const char* name);

}

#endif
