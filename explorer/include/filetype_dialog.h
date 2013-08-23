#ifndef EXPLORER_FILETYPE_DIALOG_H_
#define EXPLORER_FILETYPE_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace explorer
{

namespace view
{

class FileTypeDialog : public OnyxDialog
{
    Q_OBJECT

public:
    FileTypeDialog(QWidget *parent);
    ~FileTypeDialog(void);

public:
    int exec();

private:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void changeEvent(QEvent *event);
    bool event(QEvent * event);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);
    void createLayout();

private Q_SLOTS:
    void onReturn();
    void onOkClicked(bool);
    void onCloseClicked();

private:
    QVBoxLayout layout_;
    QHBoxLayout layout_ok_;

    QButtonGroup epub_group_;
    QButtonGroup doc_group_;

    OnyxCheckBox * epub_naboo_;
    OnyxCheckBox * epub_fbreader_;

    OnyxCheckBox * doc_office_;
    OnyxCheckBox * doc_fbreader_;

    OnyxPushButton ok_;

};

}   // namespace view

}   // namespace explorer

#endif
