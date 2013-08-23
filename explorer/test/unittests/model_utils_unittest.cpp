// Copyright 2007 Company Name. All Rights Reserved.
// Author: hjiang@dev-gems.com (Hong Jiang)

#include "onyx/base/base.h"
#include "testing/testing.h"
#include "node.h"
#include "model_util.h"
#include "common_unittest.h"

using namespace explorer::model;
using namespace common_unittest;

namespace
{

static const base::string path = "model_utils_unittest";

TEST(InvalideNode)
{
    NodeCreateContext ctx;
    ctx.type = NODE_TYPE_NULL;
    scoped_ptr<Node> node(CreateNode(NULL, ctx));
    EXPECT_NULL(node);
    ctx.type = static_cast<NodeType>(9999);
    node.reset(CreateNode(NULL, ctx));
    EXPECT_NULL(node);
}

TEST(ValidNode)
{
    NodeCreateContext ctx;
    ctx.type = NODE_TYPE_ROOT;
    scoped_ptr<Node> node1(CreateNode(NULL, ctx));
    EXPECT_NOTNULL(node1);
}

TEST(GetExtensionName)
{
    base::string file_name_a = "a";
    base::string ext_name_a  = ".abc";
    base::string file_a      = file_name_a + ext_name_a;
    static const FileInfo files [] =
    {
        {"a.abc", 10},
        {"b.aBc", 20},
        {"c.xyz", 10},
        {"d", 10}
    };
    static const int SIZE = sizeof(files) / sizeof(files[0]);
    static const base::string EXTENSION_NAME = "abc";

    CreateTempDirectory(path, files, SIZE);

    // Check extension name of a.
    boost::filesystem::path pa(path);
    pa /= files[0].path;
    base::string ext_a;
    GetExtensionName(pa.string(), ext_a);
    EXPECT_EQ(ext_a, EXTENSION_NAME);

    // Check extension name of b.
    boost::filesystem::path pb(path);
    pb /= files[1].path;
    base::string ext_b;
    GetExtensionName(pb.string(), ext_b);
    EXPECT_EQ(ext_b, EXTENSION_NAME);

    // Check extension name of c.
    boost::filesystem::path pc(path);
    pc /= files[2].path;
    base::string ext_c;
    GetExtensionName(pc.string(), ext_c);
    EXPECT_NE(ext_c, EXTENSION_NAME);

    // Check extension name of d.
    boost::filesystem::path pd(path);
    pd /= files[3].path;
    base::string ext_d;
    GetExtensionName(pd.string(), ext_d);
    EXPECT_NE(ext_d, EXTENSION_NAME);

    RemoveDirectory(path);
}


TEST(GetFileSize)
{
    static const FileInfo files [] =
    {
        {"a.abc", 10},
        {"b.aBc", 20},
        {"c.xyz", 10},
    };
    static const int SIZE = sizeof(files) / sizeof(files[0]);

    CreateTempDirectory(path, files, SIZE);

    // Check size.
    for(int i = 0; i < SIZE; ++i)
    {
        boost::filesystem::path p(path);
        p /= files[i].path;
        unsigned int size = GetFileSize(p.string());
        EXPECT_EQ(size, files[i].size);
    }

    RemoveDirectory(path);
}

}
