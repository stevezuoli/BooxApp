#include "onyx/ui/paging_manager.h"
#include "onyx/screen/screen_proxy.h"

namespace ui
{

PagingManager::PagingManager()
:current_page_(0)
{
}

PagingManager::PagingManager(const QVector<QWidget*>& vec, QWidget* prev, QWidget* next, int page)
: total_item_list_(vec)
, prev_arrow_(prev)
, next_arrow_(next)
, page_size_(page)
, current_page_(0)
{
}

PagingManager::~PagingManager(void)
{
}

void PagingManager::onPrevPage()
{
    if (current_page_ > 0)
    {
        --current_page_;
        resetCurrentItemList();
    }
}

void PagingManager::onNextPage()
{
    if (current_page_ < maxPage())
    {
        ++current_page_;
        resetCurrentItemList();
    }
}

void PagingManager::resetCurrentItemList()
{
    hideAllWidget();
    for(int i = current_page_*page_size_;
            i < total_item_list_.size() && 
            i < current_page_*page_size_ + page_size_;
            ++i)
    {
        total_item_list_[i]->show();
    }
    if (current_page_ != 0)
    {
        prev_arrow_->setEnabled(true);
    }
    if (current_page_ != maxPage())
    {
        next_arrow_->setEnabled(true);
    }
}

int PagingManager::maxPage()
{ 
    if (page_size_ > 0)
    {
        return total_item_list_.size() / page_size_ + ((total_item_list_.size() % page_size_ > 0) ? 1 : 0) - 1; 
    }
    else
    {
        return 0;
    }
}

void PagingManager::hideAllWidget()
{
    for(QVector<QWidget*>::iterator itr = total_item_list_.begin(); 
        itr != total_item_list_.end();
        ++itr)
    {
        (*itr)->hide();
    }
    prev_arrow_->setDisabled(true);
    next_arrow_->setDisabled(true);
}

}   // namespace ui

