#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_

#include <QtCore/QtCore>

/// Sound manager.
class SoundManager : public QObject
{
    Q_OBJECT
public:
    SoundManager();
    ~SoundManager();

public Q_SLOTS:
    int volume();
    bool setVolume(int volume);

    bool increaseVolume();
    bool decreaseVolume();

    bool mute(bool m);
    bool isMute();

private:
    void init();
    int index(int volume);

Q_SIGNALS:
    void volumeChanged(int new_volume, bool is_mute);

private:
    int volume_;
    bool mute_;
    QVector<int> volumes_;

};

#endif // SOUND_MANAGER_H_
