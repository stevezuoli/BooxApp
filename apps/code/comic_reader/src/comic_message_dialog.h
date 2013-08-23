#ifndef COMIC_SUGGEST_DIALOG_H_
#define COMIC_SUGGEST_DIALOG_H_

#include "onyx/ui/message_dialog.h"

using namespace ui;

namespace comic_reader
{

class ComicMessageDialog: public MessageDialog
{
    Q_OBJECT

public:
    ComicMessageDialog(QWidget *parent,
            const QString &text,
            QMessageBox::Icon icon = QMessageBox::Information,
            QMessageBox::StandardButtons buttons = QMessageBox::Yes|QMessageBox::No
            );
    ~ComicMessageDialog(void);
};

}   // namespace comic_reader

#endif
