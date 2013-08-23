#ifndef HEADER_BUTTON_H_
#define HEADER_BUTTON_H_

#include "onyx/ui/ui.h"
#include "node_types.h"

using namespace explorer::model;

class HeaderButton : public QWidget
{
    Q_OBJECT

public:
    HeaderButton(QWidget *parent, const Field field);
    ~HeaderButton(void);

public:
    inline const Field field() const { return field_; }
    void setText(const QString &title);
    void setOrder(SortOrder order);

Q_SIGNALS:
    void clicked(Field field, SortOrder order_);
    void sizeChanged(Field field, QSize s);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *);

private:
    void updateLayout();
    QSize iconActualSize();

private:
    int layout_width_;
    int layout_height_;
    QTextLayout title_layout_;

    Field field_;
    SortOrder order_;

    static QIcon ascending_icon_;
    static QIcon descending_icon_;
    static QPoint icon_pos_;

    bool is_dirty_;
};

#endif
