
#ifndef ONYX_PAGINATOR_H_
#define ONYX_PAGINATOR_H_

namespace ui
{

class Paginator
{
public:
    Paginator();
    ~Paginator();

public:
    void reset(int first, int items_per_page, int total);

    bool prev();
    bool next();
    bool jump(int new_page);

    bool isPrevEnable();
    bool isNextEnable();

    void resize(int new_items_per_page);

    int currentPage();
    int pages();

    int cursor() { return cursor_; }
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();

    int first_visible() { return first_visible_; }
    int last_visible();

    int offsetInCurrentPage();

    int size() { return size_; }
    int items_per_page() { return items_per_page_; }

    int rows() { return rows_; }
    int cols() { return cols_; }
    void setGrid(const int r, const int c) { rows_ = r; cols_ = c; }

private:
    int cursor_;            ///< Absolute position in list.
    int first_visible_;     ///< Absolute position in list.
    int items_per_page_;
    int size_;
    int rows_;
    int cols_;
};

}

#endif      // ONYX_PAGINATOR_H_
