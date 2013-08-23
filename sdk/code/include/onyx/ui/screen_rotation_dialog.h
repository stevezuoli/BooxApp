#ifndef SCREEN_ROTATION_DIALOG_H
#define SCREEN_ROTATION_DIALOG_H

#include "onyx_dialog.h"
#include "onyx/ui/catalog_view.h"

namespace ui
{

class RotationWidget : public QWidget
{
    Q_OBJECT
public:
    RotationWidget(QWidget *parent=0);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    bool findFont(QString name);

    QString fontName;
};


class ScreenRotationDialog : public OnyxDialog
{
    Q_OBJECT
public:
    ScreenRotationDialog(QWidget *parent=0);
    ~ScreenRotationDialog(void);

    int popup();

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private:
    void createLayout();
    enum Type { UP, DOWN, LEFT, RIGHT};
    void rotate(Type t);

private:
    RotationWidget rotation_widget_;
    Type rotation_;

private:
    NO_COPY_AND_ASSIGN(ScreenRotationDialog);
};

}   // namespace ui

#endif // SCREEN_ROTATION_DIALOG_H
