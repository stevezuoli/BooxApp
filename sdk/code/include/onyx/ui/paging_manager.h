#ifndef OYNX_UI_PAGING_MANAGER_H_
#define OYNX_UI_PAGING_MANAGER_H_

#include <QtGui/QtGui>
#include "buttons.h"
namespace ui
{

/// display QWidget with paging
/// 
class PagingManager : public QWidget 
{
    Q_OBJECT

public:
    PagingManager();
    PagingManager(const QVector<QWidget*>& vec, QWidget* prev, QWidget* next, int page);
    ~PagingManager(void);

public Q_SLOTS:
    void setPageSize(int size) { page_size_ = size; }
    void addItem(QWidget* item) {total_item_list_.append(item); }
    void setItemList(const QVector<QWidget*>& vec) { total_item_list_ = vec; }
    void setPageButton(QWidget* prev, QWidget* next) { prev_arrow_ = prev; next_arrow_ = next; }
    void startPaging() { resetCurrentItemList(); }
    void onPrevPage();
    void onPrevPage(bool) { onPrevPage(); }
    void onNextPage();
    void onNextPage(bool) { onNextPage(); }

private:
    int maxPage();
    void hideAllWidget();
    void resetCurrentItemList();

private:
    QVector<QWidget*> total_item_list_;
    QWidget* prev_arrow_;
    QWidget* next_arrow_;
    int page_size_;
    int current_page_;
};

}   // namespace ui

#endif

