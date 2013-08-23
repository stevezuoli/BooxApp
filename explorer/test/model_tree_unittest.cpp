// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#include <iostream>
#include "onyx/base/base.h"
#include "testing/testing.h"
#include "node.h"
#include "model_tree.h"

using namespace explorer::model;

namespace
{

TEST(ModelTree)
{
    ModelTree model;

    BranchNode *branch  = model.cdBranch(NODE_TYPE_LIBRARY);
    EXPECT_TRUE(branch);

    FolderNode *flash = model.folderNode(branch);
    NodePtrs & all = flash->mutable_children();

    for(NodePtrs::iterator it = all.begin(); it != all.end(); ++it)
    {
        if ((*it)->type() == NODE_TYPE_DIRECTORY)
        {
            EXPECT_TRUE(flash->cd((*it)->name()));
            EXPECT_TRUE(flash->cdUp());
            EXPECT_FALSE(flash->cdUp());
        }
        /*
        else if ((*it)->type() == NODE_TYPE_FILE)
        {
            FileNode *file = explorer::model::down_cast<FileNode *>(*it);
            qDebug("file %s", qPrintable(file->absolutePath()));
            qDebug("name %s", qPrintable(file->name()));
            qDebug("ext  %s", qPrintable(file->completeSuffix()));
        }
        */
    }
    EXPECT_TRUE(flash->sort(NAME, ASCENDING));
    EXPECT_TRUE(flash->sort(NAME, DESCENDING));

    EXPECT_TRUE(flash->sort(SIZE, ASCENDING));
    EXPECT_TRUE(flash->sort(SIZE, DESCENDING));

    EXPECT_TRUE(flash->sort(RATING, ASCENDING));
    EXPECT_TRUE(flash->sort(RATING, DESCENDING));

    EXPECT_TRUE(flash->sort(LAST_ACCESS_TIME, ASCENDING));
    EXPECT_TRUE(flash->sort(LAST_ACCESS_TIME, DESCENDING));


    model.cdDesktop();
    BranchNode *sd     = model.cdBranch(NODE_TYPE_SD);
    EXPECT_TRUE(sd);

}

TEST(BranchSearch)
{
    int argc  = 0;
    QApplication app(argc, 0);
    // TODO, so far, we have to dump them.
    ModelTree model;

    BranchNode *sd  = model.cdBranch(NODE_TYPE_SD);
    EXPECT_TRUE(sd);
    NodePtrs & all = sd->mutable_children();

    for(NodePtrs::iterator it = all.begin(); it != all.end(); ++it)
    {
        qDebug("%s", qPrintable((*it)->absolute_path()));
    }

    QStringList filters;
    filters << "*txt";
    bool stop = false;
    sd->search(filters, true, stop);
    all = sd->mutable_children();
    for(NodePtrs::iterator it = all.begin(); it != all.end(); ++it)
    {
        qDebug("%s", qPrintable((*it)->absolute_path()));
    }

}

}
