#ifndef ONYX_KEYBOARD_WIDGET_H_
#define ONYX_KEYBOARD_WIDGET_H_

#include "onyx/base/base.h"
#include "ui_global.h"

#include "catalog_view.h"
#include "keyboard_data_factory.h"

namespace handwriting
{
class OnyxHandwritingWidget;
};

namespace ui
{

class KeyboardData;

class OnyxKeyboard : public QWidget
{
    Q_OBJECT

public:
    enum KeyboardMenuType
    {
        KEYBOARD_MENU_SHIFT = 1,
        KEYBOARD_MENU_SYMBOL = 2,
        KEYBOARD_MENU_LANGUAGE = 3,
        KEYBOARD_MENU_WRITE = 4,
        KEYBOARD_MENU_OK = 5,
        KEYBOARD_MENU_CLEAR = 6,
    };
    static const int CATALOG_MARGIN = 1;

public:
    explicit OnyxKeyboard(QWidget *parent = 0);
    ~OnyxKeyboard();

    inline CatalogView * top() { return &top_; }
    inline CatalogView * menu() { return &menu_; }

    void initFocus();
    void setKeyboardLanguae(QLocale language);

Q_SIGNALS:
    void outOfUp(CatalogView *, int row, int col);
    void outOfDown(CatalogView *, int row, int col);

protected:
    void init(const QLocale & locale);
    bool event(QEvent * event);

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item, int user_data);

    void onOutOfUp(CatalogView *, int row, int col);
    void onOutOfDown(CatalogView *, int row, int col);

    void onViewKeyRelease(CatalogView *view, QKeyEvent *key);

private Q_SLOTS:
    void onShowKeyboard();
    void onHandwritingKeyPressed(const QString &key_text, const int &key_code);

private:
    void createLayout();
    void connectWithChildren();
    void createTop();
    void createLeft();
    void createMiddle();
    void createRight();
    void createBottom();
    void createMenu();

    void menuItemActivated(ContentView *item, int user_data);
    void resetData(bool shift);
    void shiftClicked();
    void symbolClicked();
    void languageClicked();
    void writeFunctionClicked();
    ODatas & changeCase(ODatas & datas, bool is_caps_lock);

private:
    QVBoxLayout big_layout_;
    QHBoxLayout layout_;

    CatalogView top_;
    CatalogView left_;
    CatalogView middle_;
    CatalogView right_;
    CatalogView bottom_;
    CatalogView menu_;

    scoped_ptr<handwriting::OnyxHandwritingWidget> handwriting_widget_;

    KeyboardDataFactory keyboard_data_factory_;
    KeyboardData *keyboard_data_;

    bool shift_;
    bool symbol_;
    QLocale language_;               // current language
    bool is_handwriting_;            // is handwriting state

};

}   // namespace ui

#endif
