// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "testing/testing.h"
#include "node.h"
#include "file_node.h"
#include "model_util.h"
#include "common_unittest.h"

using namespace explorer::model;
using namespace common_unittest;

namespace
{

TEST(ChildrenNodeSize)
{
    NodeCreateContext ctx;
    ctx.type = NODE_TYPE_FILE;
    scoped_ptr<Node> node(CreateNode(NULL, ctx));
    EXPECT_EQ(node->children().size(), 0);
}

TEST(ChildrenNodeShare)
{
    NodeCreateContext ctx;
    ctx.type = NODE_TYPE_FILE;
    scoped_ptr<Node> a(CreateNode(NULL, ctx));
    scoped_ptr<Node> b(CreateNode(NULL, ctx));
    EXPECT_EQ(&a->children(), &b->children());
}

TEST(FileNodeInfo)
{
    static const base::string file_name = "temp_for_unittest";
    unsigned int size = rand() * 1024 / RAND_MAX;
    CreateTempFile(file_name, size);

    NodeCreateContext ctx;
    ctx.type = NODE_TYPE_FILE;
    ctx.name = file_name;
    scoped_ptr<Node> a(CreateNode(NULL, ctx));

    // Check name and size.
    EXPECT_EQ(a->name(), file_name);
    EXPECT_EQ(down_cast<FileNode *>(a)->GetFileSize(), size);

    RemoveFile(file_name);
}

}
