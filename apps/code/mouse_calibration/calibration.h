

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QtGui/QtGui>
#include "onyx/sys/sys_status.h"
#ifdef BUILD_FOR_ARM
#include <QWSPointerCalibrationData>
#endif

/// LGPL now.
class Calibration : public QDialog
{
    Q_OBJECT
public:
    Calibration();
    ~Calibration();
    int exec();

protected:
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    void accept();
    bool event(QEvent *e);

private:
    void calculateLayout();
    bool verify();
    void resetCalibration();

private:
#ifdef BUILD_FOR_ARM
    QWSPointerCalibrationData data;
#endif
    QTextLayout layout_;
    SysStatus & sys_;
    int press_count_;
    int width_;
    int height_;
    int dx_;
    int dy_;

};



#endif
