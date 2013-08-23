#ifndef ONYX_LIB_TTS_WIDGET_H_
#define ONYX_LIB_TTS_WIDGET_H_

#include "onyx/base/base.h"
#include "onyx/sound/sound.h"
#include "onyx/tts/tts_interface.h"
#include "onyx/tts/tts.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/tts_actions.h"
#include "onyx/ui/volume_control.h"

#include "onyx/ui/catalog_view.h"
#include "onyx/ui/content_view.h"
#include "onyx/ui/factory.h"
#include "onyx/data/data_tags.h"

using namespace ui;

namespace tts
{

/// TTS widget.
class TTSWidget : public QDialog
{
    Q_OBJECT

public:
    TTSWidget(QWidget *parent, TTS & ref);
    ~TTSWidget();

public:
    TTS_State state() { return tts_.state(); }

    void setData(const QVariant & data) { data_ = data; }
    QVariant & data() { return data_; }

    void ensureVisible();

public Q_SLOTS:
    bool speak(const QString & text);
    bool pause();
    bool resume();
    bool toggle();
    bool stop();
    void changeVolume(int, bool);
    void onCloseClicked(bool);

    void closeVolumeDialog();

Q_SIGNALS:
    void speakDone();
    void closed();

private Q_SLOTS:
    void onPlayClicked(bool);
    void startPlaying();
    void onTextPlayed();
    void onPopupMenu(bool);
    void onSystemVolumeChanged(int value, bool muted);
    void onTTSInitError();

    void onVolumeButtonsPressed(bool);

    void onItemActivated(CatalogView *, ContentView *, int);

private:
    bool event(QEvent *e);
    void moveEvent(QMoveEvent *e);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent* e);

    void createLayout();
    void updateActions();

    VolumeControlDialog * volumeDialog(bool create);

private:
    TTS & tts_;
    QHBoxLayout   layout_;

    QVariant   data_;
    QString    text_;
    bool       update_parent_;

    ui::TTSSpeakerActions speaker_actions_;
    ui::TTSSpeedActions   speed_actions_;
    ui::TTSStyleActions   style_actions_;

    enum {MENU_, PLAY_, VOLUME_, CLOSE_};
    QPixmap play_icon;
    QPixmap stop_icon;
    OData *play_data;
    CatalogView         buttons_;
    scoped_ptr<VolumeControlDialog> volume_dialog_;

};

}   // namespace tts

#endif  // ONYX_LIB_TTS_WIDGET_H_
