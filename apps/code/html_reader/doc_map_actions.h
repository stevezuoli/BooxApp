#ifndef DOCUMENT_MAP_ACTIONS_H_
#define DOCUMENT_MAP_ACTIONS_H_

#include "onyx/base/base.h"
#include "onyx/ui/context_dialog_base.h"

using namespace ui;

enum IndexType
{
    INDEX_INVALID,
    INDEX_TOC,
    INDEX_INDEX,
};

class DocMapActions : public BaseActions
{
public:
    DocMapActions(void);
    ~DocMapActions(void);

public:
    /// Generate or re-generate the index type.
    void generateActions();

    /// Retrieve the selected action.
    IndexType selected();

private:
};


#endif //  DOCUMENT_MAP_ACTIONS_H_
