#ifndef EXPLORER_FONTMANAGEMENT_DIALOG_H_
#define EXPLORER_FONTMANAGEMENT_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace explorer
{

namespace view
{

class FontManagementDialog : public OnyxDialog
{
    Q_OBJECT

public:
    FontManagementDialog(QWidget *parent,  sys::SystemConfig & ref);
    ~FontManagementDialog(void);

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
    QStringList getFontFamilyList();

private Q_SLOTS:
    void onReturn();
    void onFontManagementButtonClicked(bool);
    void onOkClicked(bool);
    void onCloseClicked();

private:
    sys::SystemConfig & conf_;
    QVBoxLayout ver_layout_top_;
    QHBoxLayout hor_layout_top_;
    QVBoxLayout ver_layout_left_;
//    QVBoxLayout ver_layout_right_;
    QHBoxLayout hor_layout_sample_;
    QHBoxLayout hor_layout_ok_;

    QString font_family_;
    QStringList font_family_list_;
    QTextEdit text_;

    typedef OnyxCheckBox * CheckBoxPtr;
    typedef std::vector<CheckBoxPtr> Buttons;
    typedef std::vector<CheckBoxPtr>::iterator ButtonsIter;
    Buttons FontManagement_buttons_;
    OnyxPushButton ok_;
};

}   // namespace view

}   // namespace explorer

#endif
