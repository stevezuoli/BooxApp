#ifndef TEXT_EDIT_H_
#define TEXT_EDIT_H_

#include "onyx/ui/ui.h"

using namespace ui;

namespace text_editor
{

class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent = 0);
    ~TextEdit();

protected:
    virtual void mousePressEvent(QMouseEvent *me);
    virtual void mouseReleaseEvent(QMouseEvent *me);
    virtual void mouseDoubleClickEvent(QMouseEvent *me);

    virtual void keyPressEvent(QKeyEvent * ke);
    virtual void keyReleaseEvent(QKeyEvent * ke);

    virtual void dropEvent(QDropEvent * de);

private:
    QPoint pressed_point_;

private:
    NO_COPY_AND_ASSIGN(TextEdit);
};

};  // namespace text_edit

#endif
