#include "filetype_dialog.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/text_layout.h"
#include "onyx/screen/screen_proxy.h"
#include "system_controller.h"

namespace explorer
{

namespace view
{

FileTypeDialog::FileTypeDialog(QWidget *parent)
: OnyxDialog(parent)
, layout_(&content_widget_)
, layout_ok_(0)
, epub_group_(0)
, doc_group_(0)
, epub_naboo_(0)
, epub_fbreader_(0)
, doc_office_(0)
, doc_fbreader_(0)
, ok_(QApplication::tr("OK"), 0)
{
    setModal(true);
    setFixedSize(300, 400);
    createLayout();
}

FileTypeDialog::~FileTypeDialog(void)
{
}

int FileTypeDialog::exec()
{
    updateTitle(QApplication::tr("Preferred Applications"));
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()), onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void FileTypeDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void FileTypeDialog::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget * wnd = 0;
    // Check the current selected type.
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
    case Qt::Key_Right:
    case Qt::Key_Down:
        wnd = ui::moveFocus(&content_widget_, ke->key());
        if (wnd)
        {
            wnd->setFocus();
        }
        break;
    case Qt::Key_Return:
        onReturn();
        break;
    case Qt::Key_Escape:
        reject();
        break;
    }
}

void FileTypeDialog::changeEvent(QEvent *event)
{
   QWidget::changeEvent(event);
}

void FileTypeDialog::createLayout()
{
    // Title
    updateTitleIcon(QPixmap());
    content_widget_.setBackgroundRole(QPalette::Button);

    {
        epub_naboo_ = new OnyxCheckBox(QString(tr("Pdf Reader")),0);
        epub_fbreader_ = new OnyxCheckBox(QString(tr("FBReader")),0);
        epub_group_.addButton(epub_naboo_);
        epub_group_.addButton(epub_fbreader_);

        QVBoxLayout *vbox = new QVBoxLayout(0);
        OnyxLabel * label = new OnyxLabel(tr("ePub"),0);
        vbox->addWidget(label);
        vbox->addWidget(epub_naboo_);
        vbox->addWidget(epub_fbreader_);

        layout_.addLayout(vbox);
        layout_.addStretch(0);

        Service & epubService = controller::SystemController::instance().getEpubService();
        Service & naboo = controller::SystemController::instance().nabooReaderService();
        Service & fbreader = controller::SystemController::instance().onyxReaderService(); 
        qDebug("getEpubService:%s",qPrintable(epubService.app_name()));

        if (epubService == naboo)
        {
            epub_naboo_->setChecked(true);
        }

        if (epubService == fbreader)
        {
            epub_fbreader_->setChecked(true);
        }
    }

    Service & office_viewer = controller::SystemController::instance().officeViewerService();
    if (!office_viewer.service_name().isEmpty())
    {
        doc_office_ = new OnyxCheckBox(QString(tr("Office Reader")),0);
        doc_fbreader_ = new OnyxCheckBox(QString(tr("FBReader")),0);
        doc_group_.addButton(doc_office_);
        doc_group_.addButton(doc_fbreader_);

        QVBoxLayout *vbox = new QVBoxLayout(0);
        OnyxLabel * label = new OnyxLabel(tr("Doc"),0);
        vbox->addWidget(label);
        vbox->addWidget(doc_office_);
        vbox->addWidget(doc_fbreader_);

        layout_.addLayout(vbox);
        layout_.addStretch(0);

        Service & docService = controller::SystemController::instance().getDocService();
        Service & fbreader = controller::SystemController::instance().onyxReaderService(); 
        qDebug("getDocService:%s",qPrintable(docService.app_name()));

        if (docService == office_viewer)
        {
            doc_office_->setChecked(true);
        }

        if (docService == fbreader)
        {
            doc_fbreader_->setChecked(true);
        }

    }
    else
    {
        setFixedSize(300, 250);
    }

    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));
    ok_.useDefaultHeight();
    ok_.setCheckable(false);
    ok_.setFocusPolicy(Qt::TabFocus);
    
    layout_ok_.addStretch(0);
    layout_ok_.addWidget(&ok_);
    layout_.addLayout(&layout_ok_);
}

void FileTypeDialog::onReturn()
{
    QApplication::processEvents();
}


void FileTypeDialog::onOkClicked(bool)
{
    if (epub_naboo_->isChecked())
    {
        Service & naboo = controller::SystemController::instance().nabooReaderService();
        controller::SystemController::instance().setEpubService(naboo);
    }
    if (epub_fbreader_->isChecked())
    {
        Service & fbreader = controller::SystemController::instance().onyxReaderService(); 
        controller::SystemController::instance().setEpubService(fbreader);
    }

    if (doc_office_ && doc_office_->isChecked())
    {
        Service & office_viewer = controller::SystemController::instance().officeViewerService(); 
        controller::SystemController::instance().setDocService(office_viewer);
    }
    if (doc_fbreader_ && doc_fbreader_->isChecked())
    {
        Service & fbreader = controller::SystemController::instance().onyxReaderService(); 
        controller::SystemController::instance().setDocService(fbreader);
    }

    accept();
}

void FileTypeDialog::onCloseClicked()
{
    reject();
}

bool FileTypeDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        // onyx::screen::instance().sync(&shadows_.hor_shadow());
        // onyx::screen::instance().sync(&shadows_.ver_shadow());
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void FileTypeDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void FileTypeDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
}

}   // namespace view

}   // namespace explorer

