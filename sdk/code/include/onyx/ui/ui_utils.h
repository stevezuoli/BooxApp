#ifndef ONYX_UI_UTILS_H_
#define ONYX_UI_UTILS_H_

#include <QtGui/QtGui>

namespace ui
{

// Return the screen geometry when transform.
QRect screenGeometry();

bool dockWidget(QWidget *target, QWidget * container, Qt::Alignment align);

int statusBarHeight();

int defaultFontPointSize();

QPoint globalTopLeft(QWidget *);

QPoint globalCenter(QWidget *);

int distance(QWidget * first, QWidget *second);

int distance(QPoint first, QPoint second);

int keyboardKeyHeight();

// the width of the check-box in CheckBoxView
int checkBoxViewWidth();

// If the parent_widget is null, return the desktop widget.
QWidget * safeParentWidget(QWidget *parent_widget);

// calculates the best size for dialog
QSize bestDialogSize();

QString sizeString(int size);

bool is97inch();

bool isLandscapeVolumeMapping();

bool isHD();

};

#endif  // ONYX_UI_UTILS_H_

