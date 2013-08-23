#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qscreen_qws.h>
#endif

#include "onyx/ui/languages.h"
#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"

#include "text_frame.h"

namespace text_editor
{

static const int AUTO_SAVE_INTERVAL = 5 * 60 * 1000;

class AutoSaveHold
{
public:
    AutoSaveHold(TextFrame * parent) : parent_(parent) { parent_->auto_save_timer_.stop(); }
    ~AutoSaveHold() { parent_->auto_save_timer_.start(); }

private:
    TextFrame * parent_;
};

TextFrame::TextFrame(QObject *parent)
#ifndef Q_WS_QWS
    : QFrame(0)
#else
    : QFrame(0, Qt::FramelessWindowHint)
#endif
    , vlayout_(this)
    , hlayout_(0)
    , text_edit_(0)
    , keyboard_(0)
    , status_bar_(this, MENU | MESSAGE | BATTERY | CLOCK | SCREEN_REFRESH | INPUT_TEXT)
    , file_dialog_(0)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::WindowText);

#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

    createLayout();

    auto_save_timer_.setSingleShot(true);
    auto_save_timer_.setInterval(AUTO_SAVE_INTERVAL);
    connect(&auto_save_timer_, SIGNAL(timeout()), this, SLOT(autoSave()));
    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));
    connect(&status_bar_, SIGNAL(requestInputText()), this, SLOT(onInputText()));
#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif
}

TextFrame::~TextFrame()
{
}

void TextFrame::createLayout()
{
    hlayout_.setContentsMargins(10, 0, 10, 0);
    hlayout_.setSpacing(0);
    hlayout_.addWidget(&text_edit_);

    vlayout_.setContentsMargins(0, 0, 0, 0);
    vlayout_.setSpacing(0);

    // text edit
    vlayout_.addLayout(&hlayout_);

    // keyboard.
    keyboard_.attachReceiver(this);
    vlayout_.addWidget(&keyboard_);

    // status bar
    vlayout_.addWidget(&status_bar_);

    text_edit_.installEventFilter(this);
}

void TextFrame::returnToLibrary()
{
    if (maybeSave() == -1)
    {
        // Do NOT exit when user click ESC on the dialog.
        return;
    }
    qApp->exit();

    // We found a strange dead-lock after downloading a book. Use system exit to resolve it temporarily.
    ::exit(0);
}

void TextFrame::onInputText()
{
    if (keyboard_.isHidden())
    {
        keyboard_.setVisible(true);
    }
    else
    {
        keyboard_.setVisible(false);
    }

    if (tts_view_->isVisible())
    {
        tts_view_->ensureVisible();
    }
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void TextFrame::keyReleaseEvent(QKeyEvent * ke)
{
    QWidget * wnd = 0;
    ke->accept();
    switch(ke->key())
    {
    case Qt::Key_Escape:
        {
            if ( tts_engine_ != 0 && tts_engine_->state() == TTS_PLAYING)
            {
                stopTTS();
            }
            else
            {
                returnToLibrary();
            }
        }
        break;
    case Qt::Key_A:
        {
            selectAll();
        }
        break;
    case ui::Device_Menu_Key:
        {
            popupMenu();
        }
        break;
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
        {
            wnd = ui::moveFocus(this, ke->key());
            if (wnd)
            {
                wnd->setFocus();
            }
        }
        break;
    default:
        break;
    }
}

bool TextFrame::eventFilter(QObject *obj, QEvent *event)
{
    bool ret = QFrame::eventFilter(obj, event);
    if (obj == &text_edit_ &&
        event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            keyboard_.setFocus();
            return true;
        }
    }
    return ret;
}

void TextFrame::resizeEvent(QResizeEvent * re)
{
    QFrame::resizeEvent(re);
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void TextFrame::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_CapsLock)
    {
        return;
    }

    // Disable the parent widget to update screen.
    onyx::screen::instance().enableUpdate(false);

    if (text_edit_.hasFocus() ||
        (ke->key() != Qt::Key_Down &&
         ke->key() != Qt::Key_Up &&
         ke->key() != Qt::Key_Left &&
         ke->key() != Qt::Key_Right))
    {
        QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
        QApplication::postEvent(&text_edit_, key_event);
    }

    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate(true);

    // Update the line edit.
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true);
}

bool TextFrame::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    //qDebug("main window event type %d", event->type());
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled() &&
        isActiveWindow())
    {
        if (sys::SysStatus::instance().isSystemBusy())
        {
            sys::SysStatus::instance().setSystemBusy(false);
        }
        static int count = 0;
        qDebug("Update request %d", ++count);
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        event->accept();
        return true;
    }
    return ret;
}

bool TextFrame::load(const QString & path)
{
    auto_save_timer_.start();
    if (!QFile::exists(path))
    {
        status_bar_.setMessage(QApplication::tr("Untitled"));
        return false;
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        status_bar_.setMessage(QApplication::tr("Untitled"));
        return false;
    }

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString content = codec->toUnicode(data);
    if (Qt::mightBeRichText(content))
    {
        text_edit_.setHtml(content);
    }
    else
    {
        content = QString::fromLocal8Bit(data);
        text_edit_.setPlainText(content);
    }

    setCurrentFileName(path);
    return true;
}

void TextFrame::setCurrentFileName(const QString & name)
{
    this->file_name_ = name;
    QString message = name;
    if (message.isEmpty())
    {
        message = QApplication::tr("Untitled");
    }
    else
    {
        QFileInfo file(message);
        message = file.fileName();
    }
    status_bar_.setMessage(message);
    text_edit_.document()->setModified(false);
}

int TextFrame::maybeSave(bool ask)
{
    AutoSaveHold hold(this);
    if (!text_edit_.document()->isModified())
    {
        return 1;
    }

    if (file_name_.startsWith(QLatin1String(":/")))
    {
        return 1;
    }

    if (ask)
    {
        MessageDialog dialog(QMessageBox::Information,
                             tr("Text Editor"),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Yes|QMessageBox::No);
        int ret = dialog.exec();
        if (ret == QMessageBox::NoButton)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
            return -1;
        }
        else if (ret != QMessageBox::Yes)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
            return 0;
        }
    }
    return fileSave() ? 1 : 0;
}

bool TextFrame::fileSave()
{
    if (file_name_.isEmpty())
    {
        return fileSaveAs();
    }

    QTextDocumentWriter writer(file_name_);
    QTextCodec * codec = QTextCodec::codecForLocale();
    if (codec != 0)
    {
        writer.setCodec(codec);
    }

    bool success = writer.write(text_edit_.document());
    if (success)
    {
        text_edit_.document()->setModified(false);
        qDebug("Save Document Successfully:%s", qPrintable(file_name_));
    }
    return success;
}

bool exportDirectory(QDir & dir)
{
    const QString FOLDER = QString("text editor");

#ifdef BUILD_FOR_ARM
    // Check sd card at first.
    if (sys::SysStatus::instance().isSDMounted())
    {
        dir.cd(SDMMC_ROOT);
        if (!dir.exists(FOLDER))
        {
            dir.mkdir(FOLDER);
        }
        if (dir.cd(FOLDER))
        {
            return true;
        }
    }

    // Check flash now.
    if (sys::SysStatus::instance().isFlashMounted())
    {
        dir.cd(LIBRARY_ROOT);
        if (!dir.exists(FOLDER))
        {
            dir.mkdir(FOLDER);
        }
        if (dir.cd(FOLDER))
        {
            return true;
        }
    }
#else
    dir = QDir::home();
    if (!dir.exists(FOLDER))
    {
        dir.mkdir(FOLDER);
    }
    if (dir.cd(FOLDER))
    {
        return true;
    }
#endif

    qWarning("Can not find export directory.");
    return false;
}

bool TextFrame::fileSaveAs()
{
    if ( file_dialog_ == 0 )
    {
        file_dialog_.reset( new KeyboardDialog( this, QApplication::tr("Type Document Name") ) );
    }

    int ret = file_dialog_->popup();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
    if (ret == QDialog::Accepted)
    {
        QDir dir;
        QString path = file_dialog_->inputText();
        if (path.isEmpty())
        {
            return false;
        }

        if (!(path.endsWith(".odt", Qt::CaseInsensitive) ||
              path.endsWith(".htm", Qt::CaseInsensitive) ||
              path.endsWith(".html", Qt::CaseInsensitive) ||
              path.endsWith(".txt", Qt::CaseInsensitive)))
        {
            path += ".txt"; // default
        }

        if (exportDirectory(dir))
        {
            path = dir.absoluteFilePath(path);
            path = QString::fromLocal8Bit(path.toLocal8Bit());
            setCurrentFileName(path);
            return fileSave();
        }
    }
    return false;
}

void TextFrame::selectAll()
{
    onyx::screen::instance().enableUpdate(false);
    text_edit_.selectAll();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void TextFrame::setFontFamily(const QString & font_family)
{
    QTextCharFormat format;
    format.setFontFamily(font_family);
    mergeFormatOnWordOrSelection(format);
}

void TextFrame::setFont(const QFont & font)
{
    QTextCharFormat format;
    format.setFontPointSize(font.pointSize());
    format.setFontItalic(font.italic());
    format.setFontWeight(font.weight());
    mergeFormatOnWordOrSelection(format);
}

void TextFrame::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = text_edit_.textCursor();
    if (!cursor.hasSelection())
    {
        cursor.select(QTextCursor::Document);
    }

    cursor.mergeCharFormat(format);
    text_edit_.mergeCurrentCharFormat(format);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void TextFrame::updateActions()
{
    QString font_family = text_edit_.fontFamily();
    font_family_actions_.generateActions(font_family, false);

    QFont font = text_edit_.currentFont();
    std::vector<int> size;
    font_actions_.generateActions(font, size, font.pointSize());

    std::vector<ReadingToolsType> tools;
    tools.push_back(SELECT_ALL);
    tools.push_back(SAVE_DOCUMENT);
    tools.push_back( TEXT_TO_SPEECH );
    reading_tools_actions_.generateActions(tools);

    tools.clear();
    tools.push_back(UNDO_TOOL);
    tools.push_back(REDO_TOOL);
    reading_tools_actions_.generateActions(tools, true);

    QClipboard * clip_board = QApplication::clipboard();
    tools.clear();
    tools.push_back(COPY_CONTENT);
    if (!clip_board->text().isEmpty())
    {
        tools.push_back(PASTE_CONTENT);
    }
    reading_tools_actions_.generateActions(tools, true);
    system_actions_.generateActions();
}

void TextFrame::popupMenu()
{
    ui::PopupMenu menu(this);
    updateActions();
    menu.addGroup(&font_family_actions_);
    menu.addGroup(&font_actions_);
    menu.addGroup(&reading_tools_actions_);
    menu.setSystemAction(&system_actions_);

    qDebug("Popup Menu");
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == system_actions_.category())
    {
        SystemAction system = system_actions_.selected();
        if (system == RETURN_TO_LIBRARY)
        {
            returnToLibrary();
        }
        else if (system == SCREEN_UPDATE_TYPE)
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
            onyx::screen::instance().toggleWaveform();
        }
        else if (system == MUSIC)
        {
            // Start or show music player.
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        else if (system == ROTATE_SCREEN)
        {
            rotate();
        }
        return;
    }

    if (group == font_family_actions_.category())
    {
        setFontFamily(font_family_actions_.selectedFont());
    }
    else if (group == font_actions_.category())
    {
        setFont(font_actions_.selectedFont());
    }
    else if (group == reading_tools_actions_.category())
    {
        if (reading_tools_actions_.selectedTool() == SELECT_ALL)
        {
            selectAll();
        }
        else if (reading_tools_actions_.selectedTool() == SAVE_DOCUMENT)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            maybeSave(false);
        }
        else if (reading_tools_actions_.selectedTool() == UNDO_TOOL)
        {
            undo();
        }
        else if (reading_tools_actions_.selectedTool() == REDO_TOOL)
        {
            redo();
        }
        else if (reading_tools_actions_.selectedTool() == TEXT_TO_SPEECH)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            startTTS();
        }
        else if (reading_tools_actions_.selectedTool() == COPY_CONTENT)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            copyToClipboard();
        }
        else if (reading_tools_actions_.selectedTool() == PASTE_CONTENT)
        {
            pasteToCurrentCursor();
        }
    }
}

static RotateDegree getSystemRotateDegree()
{
    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation();
#endif
    return static_cast<RotateDegree>(degree);
}

void TextFrame::rotate()
{
#ifndef Q_WS_QWS
    RotateDegree prev_degree = getSystemRotateDegree();
    if (prev_degree == ROTATE_0_DEGREE)
    {
        resize(800, 600);
    }
    else
    {
        resize(600, 800);
    }
#else
    SysStatus::instance().rotateScreen();
#endif
}

void TextFrame::autoSave()
{
    if (!file_name_.isEmpty())
    {
        maybeSave(false);
    }
    else
    {
        auto_save_timer_.start();
    }
}

void TextFrame::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    qDebug("Screen Resize");
    resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
}

void TextFrame::undo()
{
    text_edit_.undo();
}

void TextFrame::redo()
{
    text_edit_.redo();
}

void TextFrame::copyToClipboard()
{
    text_edit_.copy();
}

void TextFrame::pasteToCurrentCursor()
{
    text_edit_.paste();
}

void TextFrame::startTTS()
{
    if ( tts_engine_ == 0 )
    {
        tts_engine_.reset(new TTS(QLocale::system()));
    }

    if ( tts_view_ == 0 )
    {
        tts_view_.reset(new TTSWidget(&text_edit_, *tts_engine_));
        tts_view_->installEventFilter(this);
        connect(tts_view_.get(), SIGNAL(speakDone()), this, SLOT(onTTSPlayFinished()));
    }

    if (!tts_view_->isVisible())
    {
        tts_view_->ensureVisible();
        requestPlayingVoice();
    }
}

void TextFrame::onTTSPlayFinished()
{
    pauseTTS();
}

void TextFrame::stopTTS()
{
    if (tts_view_ != 0)
    {
        tts_view_->onCloseClicked(true);
    }
}

bool TextFrame::pauseTTS()
{
    if (tts_view_ != 0)
    {
        return tts_view_->pause();
    }
    return false;
}

bool TextFrame::resumeTTS()
{
    if (tts_view_ != 0)
    {
        return tts_view_->resume();
    }
    return false;
}

void TextFrame::requestPlayingVoice()
{
    QString text = text_edit_.toPlainText();
    if (!text.isEmpty() && tts_engine_ != 0 && tts_view_->isVisible())
    {
        tts_view_->speak( text );
    }
}

}
