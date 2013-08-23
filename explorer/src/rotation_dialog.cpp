
#include "onyx/ui/languages.h"
#include "onyx/ui/text_layout.h"
#include "onyx/screen/screen_proxy.h"
#include "rotation_dialog.h"

namespace explorer
{

namespace view
{


RotationDialog::RotationDialog(QWidget *parent, SysStatus & ref)
    : QDialog(parent, Qt::Popup| Qt::WindowShadeButtonHint)
    , status_(ref)
    , ver_layout_(this)
    , image_widget_layout_(0)
    , hor_layout_(0)
    , degree0_widget_(QImage(":/images/degree0.png"), 0)
    , degree90_widget_(QImage(":/images/degree90.png"), 0)
    , degree270_widget_(QImage(":/images/degree270.png"), 0)
    , selected_(0)
    , ok_("OK", this)
    , cancel_("Cancel", this)
    , image_(":/images/power.png")
    , font_("", 25)
    , fade_(0)
{
    resize(450, 600);

    setModal(true);
    createLayout();
}

RotationDialog::~RotationDialog(void)
{
}

void RotationDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void RotationDialog::keyReleaseEvent(QKeyEvent *ke)
{
    // Check the current selected type.
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
        focusPreviousChild();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
        focusNextChild();
        break;
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        reject();
        break;
    }
}

void RotationDialog::createLayout()
{
    // The big layout.
    ver_layout_.setContentsMargins(2, 50, 2, 6);

    image_widget_layout_.setSpacing(3);
    image_widget_layout_.addWidget(&degree0_widget_);
    image_widget_layout_.addWidget(&degree90_widget_);
    image_widget_layout_.addWidget(&degree270_widget_);
    connect(&degree0_widget_, SIGNAL(clicked(ImageWidget*)), this, SLOT(onButtonClicked(ImageWidget*)));
    connect(&degree90_widget_, SIGNAL(clicked(ImageWidget*)), this, SLOT(onButtonClicked(ImageWidget*)));
    connect(&degree270_widget_, SIGNAL(clicked(ImageWidget*)), this, SLOT(onButtonClicked(ImageWidget*)));

    int degree = status_.screenTransformation();
    if (degree == 0)
    {
        selected_ = &degree0_widget_;
    }
    else if (degree == 90)
    {
        selected_ = &degree90_widget_;
    }
    else if (degree == 270)
    {
        selected_ = &degree270_widget_;
    }

    if (selected_)
    {
        selected_->setChecked(true);
    }

    // OK cancel buttons.
    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));
    connect(&cancel_, SIGNAL(clicked(bool)), this, SLOT(onCancelClicked(bool)));

    ok_.setFixedSize(65, 25);
    cancel_.setFixedSize(65, 25);
    hor_layout_.setContentsMargins(20, 0, 20, 0);
    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&ok_);
    hor_layout_.addWidget(&cancel_);

    ver_layout_.addLayout(&image_widget_layout_);
    ver_layout_.addStretch(0);
    ver_layout_.addLayout(&hor_layout_, 100);
}

void RotationDialog::resizeEvent(QResizeEvent *)
{
    QPainterPath p;
    p.addRoundedRect(rect(), 20, 20);
    QRegion maskedRegion(p.toFillPolygon().toPolygon());
    setMask(maskedRegion);
    updatePath();
}

void RotationDialog::updatePath()
{
    fade_.reset(new QLinearGradient(0, 0, 0, rect().height()));
    fade_->setColorAt(0, QColor(188, 188, 188, 255));
    fade_->setColorAt(0.01, QColor(56, 56, 56, 255));
    fade_->setColorAt(0.02, QColor(0, 0, 0, 255));
    fade_->setColorAt(0.05, QColor(0, 0, 0, 255));
    fade_->setColorAt(0.06, QColor(0xff, 0xff, 0xff, 255));
    fade_->setColorAt(0.94, QColor(0xff, 0xff, 0xff, 255));
    fade_->setColorAt(0.95, QColor(0, 0, 0, 255));
    fade_->setColorAt(0.98, QColor(0, 0, 0, 255));
    fade_->setColorAt(0.99, QColor(56, 56, 56, 255));
    fade_->setColorAt(1.0,  QColor(188, 188, 188, 255));
}

void RotationDialog::paintEvent(QPaintEvent *pe)
{
    QPainter p(this);

    QRect rc = rect().adjusted(2, 1, -2, -1);
    QPainterPath path;
    path.addRoundedRect(rc, 20, 20);
    QPen pen(QColor(0, 0, 0));
    pen.setWidth(2);
    p.setPen(pen);
    p.drawPath(path);

    p.fillPath(path, *fade_);

    p.drawImage(QPoint(20, 2), image_);

    QTextLayout layout;
    QRect text_rc(60, 2, 300, 30);
    ui::calculateSingleLineLayout(layout, font_, QApplication::tr("Screen Rotation"), Qt::AlignLeft, text_rc);
    pen.setColor(Qt::white);
    p.setPen(pen);
    layout.draw(&p, QPoint(0, 0));
}

bool RotationDialog::event(QEvent* qe)
{
    bool ret = QDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest)
    {
        static int count = 0;
        qDebug("RotationDialog update %d", count++);
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
    }
    return ret;
}

void RotationDialog::onOkClicked(bool)
{
    if (selected_ == &degree0_widget_)
    {
        status_.setScreenTransformation(0);
    }
    else if (selected_ == &degree90_widget_)
    {
        status_.setScreenTransformation(90);
    }
    if (selected_ == &degree270_widget_)
    {
        status_.setScreenTransformation(270);
    }

    onyx::screen::instance().enableUpdate(false);
    accept();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
}

void RotationDialog::onCancelClicked(bool)
{
    onyx::screen::instance().enableUpdate(false);
    reject();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
}

void RotationDialog::onButtonClicked(ImageWidget *widget)
{
    selected_ = widget;
    if (widget != &degree0_widget_)
    {
        degree0_widget_.setChecked(false);
    }
    if (widget != &degree90_widget_)
    {
        degree90_widget_.setChecked(false);
    }
    if (widget != &degree270_widget_)
    {
        degree270_widget_.setChecked(false);
    }
    update();
}

}   // namespace view

}   // namespace explorer

