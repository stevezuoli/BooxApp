#ifndef EXPLORER_WAVEFORM_DIALOG_H_
#define EXPLORER_WAVEFORM_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace explorer
{

namespace view
{

/// Waveform widget.
class WaveformDialog : public OnyxDialog
{
    Q_OBJECT

public:
    WaveformDialog(QWidget *parent);
    ~WaveformDialog(void);

public:
    int exec();

private:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent * event);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);
    void createLayout();

private Q_SLOTS:
    void onReturn();
    void onButtonClicked(bool);
    void onOkClicked(bool);
    void onCloseClicked();

private:
    QVBoxLayout ver_layout_;
    QHBoxLayout hor_layout_;

    typedef OnyxCheckBox * CheckBoxPtr;
    typedef std::vector<CheckBoxPtr> Buttons;
    typedef std::vector<CheckBoxPtr>::iterator ButtonsIter;
    Buttons buttons_;
    OnyxPushButton ok_;
};

}   // namespace view

}   // namespace explorer

#endif
