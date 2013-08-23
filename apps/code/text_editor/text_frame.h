#ifndef TEXT_FRAME_H_
#define TEXT_FRAME_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "onyx/sys/sys.h"

#include "onyx/tts/tts_widget.h"
#include "onyx/tts/tts.h"

#include "text_edit.h"
#include "keyboard_dialog.h"

using namespace ui;
using namespace base;
using namespace tts;

namespace text_editor
{

class TextFrame : public QFrame
{
    Q_OBJECT
public:
    TextFrame(QObject *parent = 0);
    ~TextFrame();

Q_SIGNALS:
    void rotateScreen();

public Q_SLOTS:
    bool load(const QString & path);

private Q_SLOTS:
    void popupMenu();
    void autoSave();
    void onScreenSizeChanged(int);
    void onInputText();
    void onTTSPlayFinished();

private:
    void createLayout();
    void updateActions();

    // event processor
    void keyPressEvent(QKeyEvent * ke);
    void keyReleaseEvent(QKeyEvent * ke);
    void resizeEvent(QResizeEvent * re);
    bool event(QEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);

    void returnToLibrary();

    // save
    int maybeSave(bool ask = true);
    void setCurrentFileName(const QString & name);
    bool fileSave();
    bool fileSaveAs();

    // selection
    void selectAll();

    // font
    void setFontFamily(const QString & font_family);
    void setFont(const QFont & font);
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    // rotate
    void rotate();

    // undo/redo
    void undo();
    void redo();

    // TTS
    void startTTS();
    void stopTTS();
    bool pauseTTS();
    bool resumeTTS();
    void requestPlayingVoice();

    // Copy & Paste
    void copyToClipboard();
    void pasteToCurrentCursor();

private:
    QVBoxLayout             vlayout_;           ///< Widgets Layout
    QHBoxLayout             hlayout_;           ///< Text Edit Layout
    TextEdit                text_edit_;         ///< Text Edit
    KeyBoard                keyboard_;          ///< Keyboard
    StatusBar               status_bar_;        ///< Status Bar
    scoped_ptr<KeyboardDialog> file_dialog_;

    QString                 file_name_;         ///< File Name

    // tts
    scoped_ptr<TTS>         tts_engine_;
    scoped_ptr<TTSWidget>   tts_view_;

    FontFamilyActions       font_family_actions_;    ///< Font Family
    FontActions             font_actions_;           ///< Normal Font Actions
    ReadingToolsActions     reading_tools_actions_;  ///< Reading Tools
    SystemActions           system_actions_;         ///< System Actions

    QTimer                  auto_save_timer_;

private:
    friend class AutoSaveHold;
    NO_COPY_AND_ASSIGN(TextFrame);
};

};
#endif
