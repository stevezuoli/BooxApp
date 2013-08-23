#ifndef EXPLORER_ROTATION_DIALOG_H_
#define EXPLORER_ROTATION_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"

namespace explorer
{

namespace view
{

using namespace ui;

/// Locale widget. Not sure we should place it in the ui library.
/// So far, only explorer use it.
class RotationDialog : public QDialog
{
    Q_OBJECT

public:
    RotationDialog(QWidget *parent, SysStatus & ref);
    ~RotationDialog(void);

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *pe);
    bool event(QEvent*);

    void updatePath();

private Q_SLOTS:
    void onOkClicked(bool);
    void onCancelClicked(bool);
    void onButtonClicked(ImageWidget *wnd);

private:
    SysStatus & status_;
    QVBoxLayout ver_layout_;
    QHBoxLayout image_widget_layout_;
    QHBoxLayout hor_layout_;

    ImageWidget degree0_widget_;
    ImageWidget degree90_widget_;
    ImageWidget degree270_widget_;
    ImageWidget *selected_;

    OnyxPushButton ok_;
    OnyxPushButton cancel_;

    QImage image_;
    QFont font_;
    scoped_ptr<QLinearGradient> fade_;
};

}   // namespace view

}   // namespace explorer

#endif
