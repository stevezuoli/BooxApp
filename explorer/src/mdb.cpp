#include "mdb.h"

using namespace cms;

/// All branch nodes can share the same mdb, as we use the
/// center database.
static ContentManager s_mdb_instance_;

ContentManager & mdb()
{
    if (!s_mdb_instance_.isOpen())
    {
        QString path;
        cms::getDatabasePath("", path);
        s_mdb_instance_.open(path);
    }
    return s_mdb_instance_;
}

void closeMdb()
{
    if (s_mdb_instance_.isOpen())
    {
        s_mdb_instance_.close();
    }
}

