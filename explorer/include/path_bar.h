
#ifndef EXPLORER_PATH_BAR_H_
#define EXPLORER_PATH_BAR_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "model_tree.h"

using namespace explorer::model;
using namespace ui;

class PathBarButton : public QWidget
{
    Q_OBJECT
public:
    PathBarButton(QWidget *parent);
    PathBarButton(QWidget *parent, const QString &path);
    ~PathBarButton();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public:
    const QString &text() const { return text_; }
    void setText(const QString &text);

    const QString &name() const { return name_; }
    void setName(const QString &name) { name_ = name; }

    void setChecked(bool check = true);
    bool isChecked() const { return checked_; }

Q_SIGNALS:
    void clicked (PathBarButton*);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    virtual bool event(QEvent * event);
    virtual void paintEvent(QPaintEvent * event);
    virtual void resizeEvent(QResizeEvent * event);

private:
    QString text_;
    QString name_;
    bool checked_;
    QTextLayout layout_;
    scoped_ptr<QImage> image_;
    bool layout_dirty_;
};

/// Pathbar can work in two modes: pathbar mode and titlebar mode.
class PathBar : public QWidget
{
    Q_OBJECT
public:
    PathBar(QWidget *parent,  ModelTree & model);
    ~PathBar();

public Q_SLOTS:
    bool updateAll();

Q_SIGNALS:
    void desktopClicked();
    void branchClicked(const QString &branch);
    void pathClicked(const QString &path);
    void closeClicked();

protected:
    virtual void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void onButtonClicked(PathBarButton *button);
    void onCloseClicked();

private:
    void createLayout();
    void updateLayout();
    int  maxItems();

    void setPathbarMode(bool b) { pathbar_mode_ = b; }
    bool inPathbarMode() { return pathbar_mode_; }

private:
    QString root_path_;
    QHBoxLayout layout_;
    QStringList items_;

    typedef shared_ptr<PathBarButton> ButtonPtr;
    typedef std::vector<ButtonPtr> Buttons;
    Buttons buttons_;
    PathBarButton home_;
    PathBarButton branch_;
    OnyxPushButton close_button_;
    ModelTree &tree_;
    bool pathbar_mode_;
};

#endif // EXPLORER_PATH_BAR_H_

