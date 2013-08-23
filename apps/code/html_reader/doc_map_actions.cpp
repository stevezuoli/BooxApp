#include "doc_map_actions.h"


DocMapActions::DocMapActions(void)
: BaseActions()
{
    category()->setIcon(QIcon(QPixmap(":/images/document_map.png")));
}

DocMapActions::~DocMapActions(void)
{
}

void DocMapActions::generateActions()
{
    category()->setText(QCoreApplication::tr("Document Map"));

    actions_.clear();
    shared_ptr<QAction> toc(new QAction(exclusiveGroup()));
    toc->setText(QCoreApplication::tr("Table Of Contents"));
    toc->setIcon(QIcon(QPixmap(":/images/table_of_content.png")));
    toc->setData(INDEX_TOC);
    toc->setCheckable(true);
    toc->setChecked(false);
    actions_.push_back(toc);

    /*
    TODO: not yet.
    shared_ptr<Action> index(new QAction(exclusiveGroup()));
    index->setText(QCoreApplication::tr("Index"));
    index->setIcon(QIcon(QPixmap(":/images/index.png")));
    index->setData(INDEX_INDEX);
    index->setCheckable(true);
    index->setChecked(false);
    actions_.push_back(index);
    */
}

IndexType DocMapActions::selected()
{
    // Search for the changed actions.
    QAction * act = exclusiveGroup()->checkedAction();
    if (act)
    {
        return static_cast<IndexType>(act->data().toInt());
    }
    return INDEX_INVALID;
}



