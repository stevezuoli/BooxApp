#ifndef LINE_EDIT_VIEW_GROUP_H_
#define LINE_EDIT_VIEW_GROUP_H_

#include "onyx/base/base.h"
#include "ui_global.h"
#include "content_view.h"

namespace ui
{

class LineEditViewGroup: public QObject
{
    Q_OBJECT

public:
    explicit LineEditViewGroup();
    ~LineEditViewGroup();

    void addEdit(LineEditView *edit);
    LineEditView * edit(int id) const;
    QList<LineEditView *> editList() const;

    bool exclusive() const;
    void setExclusive(bool exclusive);

    void removeEdit(LineEditView *edit);

    LineEditView * checkedEdit();

private Q_SLOTS:
    void onCheckStateChanged(LineEditView *src);

private:
    QVector<LineEditView *> edit_view_list_;
    bool is_exclusive_;

};

}   // namespace ui

#endif
