// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#include <boost/filesystem.hpp>
#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "testing/testing.h"
#include "node.h"
#include "dir_node.h"
#include "model_util.h"
#include "common_unittest.h"


using namespace explorer::model;
using namespace common_unittest;

namespace
{

namespace fs = boost::filesystem;
static const fs::path path_name =
    fs::current_path() / "tmp_directory_for_dir_node_unittest";


TEST(ChildrenNodeSize)
{
    static const FileInfo tmp_files[] =
    {
        {"a", 10},
        {"b", 20},
        {"c", 30},
        {"d", 40}
    };
    static const int size = sizeof(tmp_files) / sizeof(tmp_files[0]);
    CreateTempDirectory(path_name, tmp_files, size);

    // Create dir node.
    NodeCreateContext ctx;
    ctx.name = path_name.string();
    ctx.type = NODE_TYPE_DIRECTORY;
    scoped_ptr<Node> node(CreateNode(NULL, ctx));
    EXPECT_EQ(node->children().size(), size);
}

TEST(GenerateChildren)
{
    static const FileInfo tmp_files[] =
    {
        {"a", 10},
        {"b", 20},
        {"c", 30},
        {"d", 40},
        {"e", 20},
    };
    static const int size = sizeof(tmp_files) / sizeof(tmp_files[0]);
    CreateTempDirectory(path_name, tmp_files, size - 1);

    // Create dir node.
    NodeCreateContext ctx;
    ctx.name = path_name.string();
    ctx.type = NODE_TYPE_DIRECTORY;
    scoped_ptr<Node> node(CreateNode(NULL, ctx));
    EXPECT_EQ(node->children().size(), size - 1);

    // Create a new file
    CreateTempDirectory(path_name, &tmp_files[size - 1], 1);

    // Force the dir node to scan again.
    down_cast<DirNode *>(node)->GenerateChildren();
    EXPECT_EQ(node->children().size(), 1);
}

TEST(SortChildren)
{
    static const FileInfo tmp_files[] =
    {
        {"a", 10},
        {"b", 20},
        {"c", 30},
        {"d", 40},
        {"e", 50},
    };
    static const int size = sizeof(tmp_files) / sizeof(tmp_files[0]);
    CreateTempDirectory(path_name, tmp_files, size);

    // Create dir node.
    NodeCreateContext ctx;
    ctx.name = path_name.string();
    ctx.type = NODE_TYPE_DIRECTORY;
    scoped_ptr<Node> node(CreateNode(NULL, ctx));
    EXPECT_EQ(node->children().size(), size);

    // Sort ASCENDING.
    down_cast<DirNode *>(node)->Sort(BY_SIZE, ASCENDING);
    const NodePtrList& list_a = node->children();
    for(int i = 0; i < size; ++i)
    {
        EXPECT_EQ(list_a[i]->name(), tmp_files[i].path);
    }

    // Sort DESCENDING.
    down_cast<DirNode *>(node)->Sort(BY_SIZE, DESCENDING);
    const NodePtrList& list_b = node->children();
    for(int i = 0; i < size; ++i)
    {
        EXPECT_EQ(list_b[i]->name(), tmp_files[size - 1 - i].path);
    }
}


}
