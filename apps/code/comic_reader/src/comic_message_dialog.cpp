#include "comic_message_dialog.h"
#include "comic_utils.h"

namespace comic_reader
{

ComicMessageDialog::ComicMessageDialog(QWidget *parent,
        const QString &text,
        QMessageBox::Icon icon,
        QMessageBox::StandardButtons buttons)
    : MessageDialog(icon, COMIC_READER_APP_NAME, text, buttons, parent)
{
}

ComicMessageDialog::~ComicMessageDialog()
{
}

}   // namespace comic_reader
