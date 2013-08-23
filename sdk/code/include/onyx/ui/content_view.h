#ifndef CENTENT_VIEW_H_
#define CENTENT_VIEW_H_

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/line_edit.h"
#include "onyx/data/data.h"

namespace ui
{

class ContentView : public QWidget
{
    Q_OBJECT

public:
    ContentView(QWidget *parent);
    virtual ~ContentView();

public:
    bool updateData(OData* data, bool force_update = false);
    OData * data();
    OData * data() const;
    virtual void updateView() = 0;

    void activate(int user_data = 0);
    void repaintAndRefreshScreen();

    void setChecked(bool checked = true);
    bool isChecked();

    int penWidth() { return pen_width_; }
    void setPenWidth(int w) { pen_width_ = w; }

    void setBkColor(Qt::GlobalColor color) { bk_color_ = color; }
    Qt::GlobalColor bkColor() { return bk_color_; }

    void setFocusWaveform(onyx::screen::ScreenProxy::Waveform w);
    onyx::screen::ScreenProxy::Waveform focusWaveform();

protected:
    // set true to enable, false to disable.
    void setRepaintOnMouseRelease(bool enable = true);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *);
    void changeEvent(QEvent *event);
    void moveEvent(QMoveEvent * event);
    void resizeEvent(QResizeEvent * event);
    void paintEvent(QPaintEvent * event);
    bool event(QEvent * event);
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);

Q_SIGNALS:
    void activated(ContentView *item, int user_data);
    void keyRelease(ContentView *item, QKeyEvent *key);
    void mouse(QPoint last, QPoint current);
    void focusChanged(ContentView *item);

protected:
    bool isPressed();
    void setPressed(bool p = true);

    void drawTitle(QPainter &painter, QRect rect, int flags);

private:
    OData* data_;
    bool pressed_;
    bool checked_;
    bool repaint_on_mouse_release_;
    int pen_width_;
    Qt::GlobalColor bk_color_;
    onyx::screen::ScreenProxy::Waveform waveform_;
};


/// Cover view provides a cover and title support.
class CoverView : public ContentView
{
    Q_OBJECT

public:
    CoverView(QWidget *parent);
    virtual ~CoverView();

    static const QString type();

public:
    virtual void updateView();

protected:
    void paintEvent(QPaintEvent * event);
    void drawCover(QPainter & painter, QRect rect);
    void drawTitle(QPainter & painter, QRect rect);
};

/// CheckBox view provides a checkbox and title support.
class CheckBoxView : public ContentView
{
    Q_OBJECT

public:
    CheckBoxView(QWidget *parent);
    virtual ~CheckBoxView();

    static const QString type();

public:
    virtual void updateView();

protected:
    void paintEvent(QPaintEvent * event);
    QRect drawCheckBox(QPainter & painter, QRect rect);
    QRect drawCover(QPainter & painter, QRect rect);
    void drawTitle(QPainter & painter, QRect rect);
};


/// LineEditView.
class LineEditView : public ContentView
{
    Q_OBJECT

public:
    LineEditView(QWidget *parent);
    virtual ~LineEditView();

    static const QString type();
    inline OnyxLineEdit * innerEdit() { return &inner_edit_; }

public:
    virtual void updateView();

public Q_SLOTS:
    void checkEditByMouse();

Q_SIGNALS:
    void checkStateChanged(LineEditView *self);

protected:
    void paintEvent(QPaintEvent * event);

    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);

private Q_SLOTS:
    void onEditOutOfRange(QKeyEvent *ke);

private:
    void createLayout();

private:
    OnyxLineEdit inner_edit_;
    QHBoxLayout layout_;
    bool forward_focus_;
};


class ClockView : public ContentView
{
    Q_OBJECT

public:
    ClockView(QWidget *parent);
    virtual ~ClockView();

    static const QString type();

public:
    virtual void updateView();

protected:
    void paintEvent(QPaintEvent * event);
    void drawDigitalClock(QPainter &painter);
};

class ButtonView : public ContentView
{
        Q_OBJECT

    public:
        ButtonView(QWidget *parent);
        virtual ~ButtonView();

        static const QString type();

    public:
        virtual void updateView();

    protected:
        void paintEvent(QPaintEvent * event);

        void keyPressEvent(QKeyEvent *e);
        void keyReleaseEvent(QKeyEvent *e);

    private:
        QLabel label_title_;

        void drawTitle(QPainter & painter, QRect rect);
};

};

#endif
