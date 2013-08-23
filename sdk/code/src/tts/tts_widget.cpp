
#include "onyx/tts/tts.h"
#include "onyx/tts/tts_widget.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys.h"

using namespace ui;

namespace tts
{

TTSWidget::TTSWidget(QWidget *parent, TTS & ref)
    : QDialog(parent, Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint)
    , tts_(ref)
    , layout_(this)
    , play_icon(":/images/tts_play.png")
    , stop_icon(":/images/tts_stop.png")
    , buttons_(0)
{
    createLayout();
    setModal(false);
    setBackgroundRole(QPalette::Dark);
    setFocusPolicy(Qt::NoFocus);

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(volumeChanged(int, bool)), this, SLOT(onSystemVolumeChanged(int, bool)));
}

TTSWidget::~TTSWidget()
{
}

void TTSWidget::onTTSInitError()
{
    QString err_msg;
    if (tts_.valid() == TTS_DATA_INVALID)
    {
        err_msg = tr("Invalid TTS data!");
    }
    else if (tts_.valid() == TTS_PLUGIN_INVALID)
    {
        err_msg = tr("Can not load TTS plugin!");
    }
    else
    {
        err_msg = tr("Unknown error!");
    }
    ErrorDialog err_dialog(err_msg);
    err_dialog.exec();
    QTimer::singleShot(0, this, SLOT(close()));
}

void TTSWidget::onVolumeButtonsPressed(bool)
{
    QRegion region = visibleRegion();
    if (region.isEmpty())
    {
        return;
    }

    QRect visible_rect = region.boundingRect();
    if (visible_rect.width() < height() && visible_rect.height() < height())
    {
        return;
    }

    VolumeControlDialog * dialog = volumeDialog(true);
    if (!dialog->isVisible())
    {
        dialog->ensureVisible();
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU,
                false, onyx::screen::ScreenCommand::WAIT_COMMAND_FINISH);
    }
    else
    {
        dialog->resetTimer();
    }
}

void TTSWidget::showEvent(QShowEvent* event)
{
    if(tts_.valid() == TTS_DATA_INVALID || tts_.valid() == TTS_PLUGIN_INVALID)
    {
        event->ignore();
        onTTSInitError();
    }
}

void TTSWidget::ensureVisible()
{
    if (!isVisible())
    {
        show();
    }

    QRect parent_rect = parentWidget()->rect();
    int border = (frameGeometry().width() - geometry().width());

    // Check position.
    QPoint new_pos(border, border);
    new_pos.ry() = parent_rect.height() - height() - border * 2;
    update_parent_ = true;
    if (pos() != new_pos)
    {
        move(new_pos);
    }

    qDebug() << "buttons height: " << buttons_.height() <<
                "buttons width: " << buttons_.width();

    // Make sure the widget is visible.
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GU);
}

void TTSWidget::onSystemVolumeChanged(int value, bool muted)
{
    qDebug("Volume Change:%d", value);
    tts_.sound().setVolume(value);
}

bool TTSWidget::speak(const QString & text)
{
    // Make sure the widget is visible.
    onyx::screen::instance().ensureUpdateFinished();

    // Remember the message.
    text_ = text;
    startPlaying();
    return true;
}

void TTSWidget::startPlaying()
{
    tts::TTS_State prev_state = tts_.state();
    tts_.setState(tts::TTS_PLAYING);

    //if (prev_state == tts::TTS_STOPPED)
    {
        tts_.speak(text_);
    }
    /*else if (prev_state == tts::TTS_PAUSED)
    {
        resume();
    }*/

    play_data->insert(TAG_COVER, stop_icon);
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
}

bool TTSWidget::pause()
{
    play_data->insert(TAG_COVER, play_icon);
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
    return tts_.pause();
}

bool TTSWidget::resume()
{
    play_data->insert(TAG_COVER, stop_icon);
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
    return tts_.resume();
}

bool TTSWidget::toggle()
{
    return tts_.toggle();
}

bool TTSWidget::stop()
{
    play_data->insert(TAG_COVER, play_icon);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
    return tts_.stop();
}

void TTSWidget::changeVolume(int v, bool m)
{
    tts_.changeVolume(v, m);
}

bool TTSWidget::event(QEvent *e)
{
    int ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        static int count = 0;
        qDebug("tts widget update %d", ++count);
        if (update_parent_)
        {
            onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GC);
            update_parent_ = false;
        }
        else
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        }
        e->accept();
    }
    return ret;
}

void TTSWidget::moveEvent(QMoveEvent *e)
{
    update_parent_ = true;
}

void TTSWidget::resizeEvent(QResizeEvent *e)
{
    update_parent_ = true;
}

void TTSWidget::createLayout()
{
    // top box
    layout_.setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout_.setContentsMargins(2, 2, 2, 2);
    layout_.setSpacing(10);

    ODatas button_data;
    buttons_.setSubItemType(CoverView::type());
    buttons_.setPreferItemSize(play_icon.size());

    // menu button.
    {
        OData * dd = new OData;
        QPixmap menu_map(":/images/tts_menu.png");
        dd->insert(TAG_COVER, menu_map);
        dd->insert(TAG_ID, MENU_);
        button_data.push_back(dd);
    }

    // play button
    {
        play_data = new OData;
        play_data->insert(TAG_COVER, play_icon);
        play_data->insert(TAG_ID, PLAY_);
        button_data.push_back(play_data);
    }

    // volume button
    {
        OData * dd = new OData;
        QPixmap volume_map(":/images/tts_volume.png");
        dd->insert(TAG_COVER, volume_map);
        dd->insert(TAG_ID, VOLUME_);
        button_data.push_back(dd);
    }

    // close button
    {
        OData * dd = new OData;
        QPixmap close_map(":/images/tts_close.png");
        dd->insert(TAG_COVER, close_map);
        dd->insert(TAG_ID, CLOSE_);
        button_data.push_back(dd);
    }

    buttons_.setData(button_data);
    buttons_.setFixedGrid(1, 4);
    buttons_.setSpacing(5);
    buttons_.setMinimumHeight(play_icon.height()+10);
    buttons_.setMinimumWidth(play_icon.width()*4+30);
    qDebug() << "min height: " << buttons_.minimumHeight() <<
                "min width: " << buttons_.minimumWidth();
    buttons_.setCheckedTo(0, 0);

    // Setup connection.
    connect(&tts_, SIGNAL(speakDone()), this, SLOT(onTextPlayed()));

    connect(&buttons_, SIGNAL(itemActivated(CatalogView*,ContentView*,int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)), Qt::QueuedConnection);

    layout_.addWidget(&buttons_);
}

void TTSWidget::onPlayClicked(bool)
{
    tts_.toggle();
    if (state() == TTS_PLAYING)
    {
        startPlaying();
    }
    else if (state() == TTS_PAUSED)
    {
        pause();
    }
}

void TTSWidget::updateActions()
{
    QStringList speakers;
    if (tts_.speakers(speakers))
    {
        QString current_speaker;
        tts_.currentSpeaker(current_speaker);
        speaker_actions_.generateActions(speakers, current_speaker);
    }

    QVector<int> speeds;
    if (tts_.speeds(speeds))
    {
        int current_speed = 2;
        tts_.currentSpeed(current_speed);
        speed_actions_.generateActions(speeds, current_speed);
    }

    QVector<int> styles;
    if (tts_.styles(styles))
    {
        int current_style = SPEAK_STYLE_NORMAL;
        tts_.currentStyle(current_style);
        style_actions_.generateActions(styles, current_style);
    }
}

VolumeControlDialog *TTSWidget::volumeDialog(bool create)
{
    if (!volume_dialog_ && create)
    {
        volume_dialog_.reset(new VolumeControlDialog(0, 3500));
    }
    return volume_dialog_.get();
}

void TTSWidget::closeVolumeDialog()
{
    VolumeControlDialog *dialog = volumeDialog(false);
    if (dialog)
    {
        dialog->reject();
        volume_dialog_.reset(0);
    }
}

void TTSWidget::onPopupMenu(bool)
{
    // Make sure the display update is finished, otherwise
    // user can not see the menu on the screen.
    onyx::screen::instance().ensureUpdateFinished();
    ui::PopupMenu menu(0);
    updateActions();
    menu.addGroup(&speaker_actions_);
    menu.addGroup(&speed_actions_);
    menu.addGroup(&style_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    update();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);

    QAction * group = menu.selectedCategory();
    if (group == speaker_actions_.category())
    {
        tts_.setSpeaker(speaker_actions_.selectedSpeaker());
    }
    else if (group == speed_actions_.category())
    {
        tts_.setSpeed(speed_actions_.selectedSpeed());
    }
    else if (group == style_actions_.category())
    {
        tts_.setStyle(style_actions_.selectedStyle());
    }
}

void TTSWidget::keyPressEvent(QKeyEvent *ke)
{
    // Need to ignore the escape and return key, otherwise,
    // the tts widget will also change play button state.
    int key = ke->key();
    if (key == Qt::Key_Escape || key == Qt::Key_Return || key == Qt::Key_Enter)
    {
        ke->ignore();
        return;
    }
    QDialog::keyPressEvent(ke);
}

void TTSWidget::keyReleaseEvent(QKeyEvent *ke)
{
    int key = ke->key();
    if (key == Qt::Key_Escape)
    {
        onCloseClicked(true);
        ke->ignore();
        return;
    }
    else if (key == ui::Device_Menu_Key)
    {
        onPopupMenu(true);
    }
    ke->accept();
}

void TTSWidget::onCloseClicked(bool)
{
    if (!isVisible())
    {
        return;
    }

    closeVolumeDialog();

    stop();
    emit closed();
    done(QDialog::Rejected);
}

/// On sentence played, we can play the next sentence now.
/// If there is no more sentece, playFinished signal is emitted.
void TTSWidget::onTextPlayed()
{
    // Check state.
    if (!tts_.isPlaying())
    {
        return;
    }

    emit speakDone();
}

void TTSWidget::onItemActivated(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }

    OData * item_data = item->data();
    int type = item_data->value(TAG_ID).toInt();

    switch(type)
    {
    case MENU_:
        onPopupMenu(true);
        break;
    case PLAY_:
        onPlayClicked(true);
        break;
    case VOLUME_:
        onVolumeButtonsPressed(true);
        break;
    case CLOSE_:
        onCloseClicked(true);
        break;
    default:
        break;
    }
}

}
