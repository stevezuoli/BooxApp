// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#include <iostream>
#include "onyx/base/base.h"
#include "testing/testing.h"
#include "node.h"
#include "model_tree.h"
#include "database_manager.h"

using namespace explorer::model;

namespace
{

void dump(Node *node)
{
    std::cout << "title: " << node->title() << std::endl;
    std::cout << "desc:  " << node->description() << std::endl;
    NodePtrList& children = node->mutable_children();
    for(NodePtrList::iterator iter = children.begin();
        iter != children.end();
        ++iter)
    {
        dump(*iter);
    }
}


TEST(ModelTree)
{
    ModelTree model;
    Node* current = model.GetCurrentNode();
    dump(current);
    std::cout << "done" << std::endl;
}

}
