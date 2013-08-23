#ifndef ABOUT_DIALOG_H_
#define ABOUT_DIALOG_H_

#include <QtWebKit/QtWebKit>
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/status_bar.h"

namespace ui
{

class AboutItem : public QWidget
{
public:
    AboutItem(QWidget *parent = 0);
    ~AboutItem();

public:
    void setInformation(const QString & image_path, const QString & text);

private:
    void createLayout();

private:
    QVBoxLayout layout_;
    OnyxLabel image_;
    OnyxLabel text_;
};

/// About Dialog
class AboutDialog : public ui::OnyxDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent);
    ~AboutDialog(void);

public:
    int  exec();

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent*);
    void updateEntries();

private Q_SLOTS:
    void onStatusItemClicked(const int, const int);
    void updatePage();
    bool prevPage();
    bool nextPage();

private:
    QVBoxLayout     ver_layout_;
    AboutItem item_;
    StatusBar status_bar_;
    int page_number_;
};

};  // namespace ui

#endif  // ABOUT_DIALOG_H_
