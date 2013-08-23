#include "onyx/ui/line_edit_view_group.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"

namespace ui
{

LineEditViewGroup::LineEditViewGroup()
    : is_exclusive_(true)
{

}

LineEditViewGroup::~LineEditViewGroup()
{
}

void LineEditViewGroup::addEdit(LineEditView *edit)
{
    if (edit_view_list_.contains(edit))
    {
        edit_view_list_.remove(edit_view_list_.indexOf(edit));
    }
    edit_view_list_.push_back(edit);
    connect(edit, SIGNAL(checkStateChanged(LineEditView *)),
            this, SLOT(onCheckStateChanged(LineEditView *)));
}

LineEditView * LineEditViewGroup::edit(int id) const
{
    return edit_view_list_.at(id);
}

QList<LineEditView *> LineEditViewGroup::editList() const
{
    return edit_view_list_.toList();
}

bool LineEditViewGroup::exclusive() const
{
    return is_exclusive_;
}

void LineEditViewGroup::setExclusive(bool exclusive)
{
    is_exclusive_ = exclusive;
}

void LineEditViewGroup::removeEdit(LineEditView *edit)
{
    int index = edit_view_list_.indexOf(edit);
    if (-1 != index)
    {
        edit_view_list_.remove(index);
    }
}

LineEditView * LineEditViewGroup::checkedEdit()
{
    LineEditView *checked = edit_view_list_.front();
    foreach (LineEditView *edit_item, edit_view_list_)
    {
        OData *data = edit_item->data();
        if (data && data->value(TAG_CHECKED).toBool())
        {
            checked = edit_item;
            break;
        }
    }
    return checked;
}

void LineEditViewGroup::onCheckStateChanged(LineEditView *src)
{
    if (is_exclusive_ && !src->isChecked())
    {
        if (src->data())
        {
            src->data()->insert(TAG_CHECKED, true);
        }

        foreach (LineEditView *edit_item, edit_view_list_)
        {
            if (edit_item != src && edit_item->data())
            {
                edit_item->data()->insert(TAG_CHECKED, false);
                edit_item->update();
                onyx::screen::watcher().enqueue(edit_item,
                        onyx::screen::ScreenProxy::DW,
                        onyx::screen::ScreenCommand::WAIT_NONE);
            }
        }
    }
}


}   // namespace ui
