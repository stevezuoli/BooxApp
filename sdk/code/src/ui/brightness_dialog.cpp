
#include "onyx/sys/sys.h"
#include "onyx/ui/brightness_dialog.h"
#include "onyx/ui/ui_utils.h"

namespace ui
{
static const int MY_WIDTH = 20;
static const int MY_HEIGHT = 150;

static const QString FANCY_STYLE = "                                \
    QSlider::groove:horizontal                                      \
    {                                                               \
        border: 1px solid #bbb;                                     \
        background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1,  \
                                    stop: 0 #6666e, stop: 1 #b66bf);\
        background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,   \
                                    stop: 0 #b23bf, stop: 1 #3455f);\
        height: 40px;                                               \
        border-radius: 8px;                                         \
    }                                                               \
    QSlider::sub-page:horizontal                                    \
    {                                                               \
        background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1,  \
                                    stop: 0 #e366e, stop: 1 #bbbbf);\
        background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,   \
                                    stop: 0 #bbbbf, stop: 1 #dd55f);\
        border: 1px solid #777;                                     \
        height: 40px;                                               \
        border-radius: 4px;                                         \
    }                                                               \
    QSlider::add-page:horizontal                                    \
    {                                                               \
        background: #fff;                                           \
        border: 1px solid #777;                                     \
        height: 40px;                                               \
        border-radius: 4px;                                         \
    }                                                               \
    QSlider::handle:horizontal                                      \
    {                                                               \
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1,         \
                                    stop:0 #eee, stop:1 #ccc);      \
        border: 1px solid #777;                                     \
        height: 45px;                                               \
        width: 30px;                                                \
        margin-top: -5px;                                           \
        margin-bottom: -5px;                                        \
        border-radius: 4px;                                         \
    }                                                               \
    QSlider::handle:horizontal:hover                                \
    {                                                               \
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1,         \
                                    stop:0 #fff, stop:1 #ddd);      \
        border: 1px solid #444;                                     \
        border-radius: 4px;                                         \
    }                                                               \
    QSlider::sub-page:horizontal:disabled                           \
    {                                                               \
        background: #bbb;                                           \
        border-color: #999;                                         \
    }                                                               \
    QSlider::add-page:horizontal:disabled                           \
    {                                                               \
        background: #eee;                                           \
        border-color: #999;                                         \
    }                                                               \
    QSlider::handle:horizontal:disabled                             \
    {                                                               \
        background: #eee;                                           \
        border: 1px solid #aaa;                                     \
        border-radius: 4px;                                         \
    }";



BrightnessDialog::BrightnessDialog(QWidget *parent)
: QDialog(parent, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint)
, layout_(this)
, title_(0)
, slider_(0)
{
    setModal(true);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Text);
    setWindowOpacity(0.8);

    createLayout();
    updateText();
}

BrightnessDialog::~BrightnessDialog(void)
{
}

int BrightnessDialog::exec()
{
    QWidget *p = ui::safeParentWidget(0);
    setFixedWidth(p->width());
    setFixedHeight(MY_HEIGHT);
    show();
    move(p->frameGeometry().left(), p->frameGeometry().bottom() - MY_HEIGHT);
    return QDialog::exec();
}

void BrightnessDialog::createLayout()
{
    layout_.setContentsMargins(MY_WIDTH, 2, MY_WIDTH, 2);

    title_.useTitleBarStyle();
    title_.setAlignment(Qt::AlignCenter);
    layout_.addWidget(&title_);

    slider_.setRange(10, 255);
    slider_.setOrientation(Qt::Horizontal);
    slider_.setFixedHeight(55);
    slider_.setStyleSheet(FANCY_STYLE);
    slider_.setValue(sys::SysStatus::instance().brightness());
    layout_.addWidget(&slider_);
    layout_.addSpacing(40);

    connect(&slider_, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}

void BrightnessDialog::updateText()
{
    title_.setText(tr("Brightness"));
}

void BrightnessDialog::onValueChanged(int v)
{
    qDebug() << "Value changed " << v;
    sys::SysStatus::instance().setBrightness(v);
}

void BrightnessDialog::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
}

void BrightnessDialog::keyReleaseEvent(QKeyEvent * ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Escape)
    {
        reject();
    }
}

};  // namespace ui



