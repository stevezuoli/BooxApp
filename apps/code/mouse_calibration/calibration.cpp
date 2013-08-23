#include "calibration.h"

#ifdef BUILD_FOR_ARM
#include <QWSPointerCalibrationData>
#include <QScreen>
#include <QWSServer>
#endif

#include <QPainter>
#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/message_dialog.h"

using namespace ui;

Calibration::Calibration()
: sys_(SysStatus::instance())
{
    QRect desktop = QApplication::desktop()->geometry();
    desktop.moveTo(QPoint(0, 0));
    setGeometry(desktop);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setModal(true);

#ifdef BUILD_FOR_ARM
    width_ = QScreen::instance()->deviceWidth();
    height_ = QScreen::instance()->deviceHeight();

    dx_ = width_ / 10;
    dy_ = height_ / 10;

    QPoint *points = data.screenPoints;
    points[QWSPointerCalibrationData::TopLeft] = QPoint(dx_, dy_);
    points[QWSPointerCalibrationData::BottomLeft] = QPoint(dx_, height_ - dy_);
    points[QWSPointerCalibrationData::BottomRight] = QPoint(width_ - dx_, height_ - dy_);
    points[QWSPointerCalibrationData::TopRight] = QPoint(width_ - dx_, dy_);
    points[QWSPointerCalibrationData::Center] = QPoint(width_ / 2, height_ / 2);
#endif
    press_count_ = 0;

    calculateLayout();

    // In order to receive all GUI events.
    sys_.setSystemBusy(false);
}

Calibration::~Calibration()
{
}

int Calibration::exec()
{
    onyx::screen::instance().enableUpdate(false);
    show();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC, true, onyx::screen::ScreenCommand::WAIT_ALL);

    /*
    MessageDialog confirm_dialog(QMessageBox::Information,
                                 tr("Information"),
                                 QPixmap(""),
                                 tr("Please press OK to start stylus calibration."),
                                 QMessageBox::Yes);
    confirm_dialog.show();
    QApplication::processEvents();
    confirm_dialog.exec();
    */

#ifdef BUILD_FOR_ARM
    sys_.clearCalibration();
#endif
    grabMouse();
    int ret = QDialog::exec();
    releaseMouse();
    return ret;
}

void Calibration::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
    layout_.draw(&p, QPointF());

#ifdef BUILD_FOR_ARM
    QPoint point = data.screenPoints[press_count_];

    // Map to logical coordinates in case the screen is transformed
    QSize screenSize(QScreen::instance()->deviceWidth(), QScreen::instance()->deviceHeight());
    point = QScreen::instance()->mapFromDevice(point, screenSize);


    p.fillRect(point.x() - 6, point.y() - 1, 13, 3, Qt::black);
    p.fillRect(point.x() - 1, point.y() - 6, 3, 13, Qt::black);
#endif
}

void Calibration::mouseReleaseEvent(QMouseEvent *event)
{
#ifdef BUILD_FOR_ARM
    // Map from device coordinates in case the screen is transformed
    QSize screenSize(QScreen::instance()->width(), QScreen::instance()->height());
    QPoint p = QScreen::instance()->mapToDevice(event->pos(), screenSize);
    qDebug("point %d %d", p.x(), p.y());

    data.devPoints[press_count_] = p;

    if (++press_count_ < 5)
    {
        repaint();
        return;
    }

    // Before we accept the result, verify the data
    if (verify())
    {
        accept();
    }
    else
    {
        MessageDialog error_dialog(QMessageBox::Information,
                                   tr("Information"),
                                   tr("Error detected. Please calibrate again."),
                                   QMessageBox::Yes);
        error_dialog.exec();
        press_count_ = 0;
        repaint();
    }

#endif
}

void Calibration::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void Calibration::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Escape)
    {
        // Make sure reset all calibration data, otherwise user can not use
        // stylus until reboot.
        resetCalibration();
        reject();
    }
}

void Calibration::accept()
{
    Q_ASSERT(press_count_ == 5);

#ifdef BUILD_FOR_ARM
    sys_.calibrate(data);
#endif

    QDialog::accept();
}

bool Calibration::event(QEvent *e)
{
    int ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC, true, onyx::screen::ScreenCommand::WAIT_ALL);
        e->accept();
        return true;
    }
    return ret;
}

void Calibration::calculateLayout()
{
    qreal width = rect().width() * 3 / 4;
    QFont font("", 20, QFont::Bold);
    layout_.setFont(font);
    layout_.setText(tr("Please press once at each of the marks shown in the screen."));
    layout_.beginLayout();

    qreal height = 0;
    QTextLine line = layout_.createLine();
    while (line.isValid())
    {
        line.setLineWidth(width);
        line.setPosition(QPointF((rect().width() - line.naturalTextWidth()) / 2, height));
        height += line.height();
        line = layout_.createLine();
    }
    layout_.endLayout();

    layout_.setPosition(QPointF(0, 150));
}

// Data collected from experiments
// dx 30 dy 40 width 600 height 800
//rotate 0
//top left:  event pos (756 11384) device pos (756 11384)
//bottom left: event pos (717 732) device pos (717 732)
//bottom right: event pos (8195 773) device pos (8195 773)
//top right event pos (8234 11325) device pos (8234 11325)
//center event pos (4589 6078) device pos (4589 6078)
//
//rotate 270
//event pos (11367 -199) device pos (798 11367)
//event pos (752 -178) device pos (777 752)
//event pos (755 -7747) device pos (8346 755)
//event pos (11321 -7637) device pos (8236 11321)
//event pos (6101 -3934) device pos (4533 6101)
/// Verify the user calibration data is correct or not.
/// From experiment, the input points should satify the following conditions:
/// center.x ~ (left + right) / 2 (less than 200)
/// center.y ~ (top + bottom) / 2 (less than 200)
bool Calibration::verify()
{
    // TODO, the threshold depends on the screen size.
    static const int THRESHOLD = 200;
#ifdef BUILD_FOR_ARM
    // Check the center point.
    QPoint *points = data.devPoints;
    int x1 = (points[QWSPointerCalibrationData::TopLeft].x() +
              points[QWSPointerCalibrationData::TopRight].x()) / 2;
    int x2 = (points[QWSPointerCalibrationData::BottomLeft].x() +
              points[QWSPointerCalibrationData::BottomRight].x()) / 2;

    if (abs(x1 - x2) >= THRESHOLD)
    {
        return false;
    }
    int x = points[QWSPointerCalibrationData::Center].x();
    if (abs(x1 - x) >= THRESHOLD)
    {
        return false;
    }

    int y1 = (points[QWSPointerCalibrationData::TopLeft].y() +
              points[QWSPointerCalibrationData::BottomLeft].y()) / 2;
    int y2 = (points[QWSPointerCalibrationData::TopRight].y() +
              points[QWSPointerCalibrationData::BottomRight].y()) / 2;
    if (abs(y1 - y2) >= THRESHOLD)
    {
        return false;
    }
    int y = points[QWSPointerCalibrationData::Center].y();
    if (abs(y1 - y) >= THRESHOLD)
    {
        return false;
    }

    // Check the device positon.
    qreal dist_x = points[QWSPointerCalibrationData::TopLeft].x() -
                   points[QWSPointerCalibrationData::TopRight].x();
    qreal dist_y = points[QWSPointerCalibrationData::TopLeft].y() -
                   points[QWSPointerCalibrationData::BottomLeft].y();

    qreal length_x = width_ - 2 * dx_;
    qreal length_y = height_ - 2 * dy_;

    qreal ratio_x = fabs(dist_x / length_x);
    qreal ratio_y = fabs(dist_y / length_y);

    if (fabs(ratio_x - ratio_y) >= 1.5)
    {
        return false;
    }
#endif
    return true;
}

// We use the points from experiment as the default settings.
void Calibration::resetCalibration()
{
#ifdef BUILD_FOR_ARM
    QPoint *points = data.devPoints;
    // Don't need to consider the rotation here.
    // From experiment.
    //top left:  event pos (756 11384) device pos (756 11384)
    //bottom left: event pos (717 732) device pos (717 732)
    //bottom right: event pos (8195 773) device pos (8195 773)
    //top right event pos (8234 11325) device pos (8234 11325)
    //center event pos (4589 6078) device pos (4589 6078)
    points[QWSPointerCalibrationData::TopLeft] = QPoint(949, 10924);
    points[QWSPointerCalibrationData::BottomLeft] = QPoint(964, 1124);
    points[QWSPointerCalibrationData::BottomRight] = QPoint(8258, 1053);
    points[QWSPointerCalibrationData::TopRight] = QPoint(8138, 10908);
    points[QWSPointerCalibrationData::Center] = QPoint(4598, 5997);
    sys_.calibrate(data);

    // Not really necessary to write them into the calibration file.
    // Startup script can handle that.
#endif
}


