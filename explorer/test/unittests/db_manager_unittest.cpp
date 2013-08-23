// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#include "onyx/base/base.h"
#include "testing/testing.h"
#include "node.h"
#include "model_util.h"
#include "common_unittest.h"
#include "database_manager.h"

using namespace explorer::model;
using namespace common_unittest;
using namespace cms;

namespace
{

static const base::string dir_path = "db_manager_unittest";

TEST(GetDatabase)
{
    static const FileInfo files [] =
    {
        {"a.abc", 10},
        {"b.aBc", 20},
        {"c.xyz", 10},
        {"d", 10}
    };
    static const int SIZE = sizeof(files) / sizeof(files[0]);

    base::string path =
        (boost::filesystem::current_path() / dir_path).string();
    CreateTempDirectory(path, files, SIZE);
    for(int i = 0; i < SIZE; ++i)
    {
        boost::filesystem::path p(path);
        p /= files[i].path;
        DatabaseManager::instance().GetDatabase(p.string());
    }

    RemoveDirectory(path);
}

}
