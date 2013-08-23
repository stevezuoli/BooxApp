#ifndef CONTENT_DB_H_
#define CONTENT_DB_H_

#include "onyx/cms/content_manager.h"

using namespace cms;

ContentManager & mdb();

void closeMdb();

#endif
