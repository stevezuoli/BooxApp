#include "onyx/ui/ui_utils.h"
#include "onyx/ui/onyx_keyboard_utils.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/onyx_keyboard.h"
#include "onyx/ui/keyboard_key_view.h"
#include "onyx/ui/factory.h"
#include "onyx/ui/keyboard_key_view_factory.h"
#include "onyx/ui/onyx_keyboard_language_dialog.h"
#include "onyx/ui/keyboard_data.h"
#include "onyx/ui/onyx_handwriting_widget.h"

using namespace handwriting;

namespace ui
{

static const QSize s_size(keyboardKeyHeight(), keyboardKeyHeight());

static const int LETTER_ROWS = 3;
static const int LEFT_COLS = 3;
static const int MIDDLE_COLS = 3;
static const int RIGHT_COLS = 3;

static KeyBoardKeyViewFactory keyboard_key_view_factory;

OnyxKeyboard::OnyxKeyboard(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , big_layout_(this)
    , layout_(0)
    , top_(&keyboard_key_view_factory, this)
    , left_(&keyboard_key_view_factory, this)
    , middle_(&keyboard_key_view_factory, this)
    , right_(&keyboard_key_view_factory, this)
    , bottom_(&keyboard_key_view_factory, this)
    , menu_(&keyboard_key_view_factory, this)
    , handwriting_widget_(0)
    , keyboard_data_(0)
    , shift_(false)
    , symbol_(false)
    , language_(QLocale::system())
    , is_handwriting_(false)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);

    init(language_);
    createLayout();
}

OnyxKeyboard::~OnyxKeyboard()
{
}

void OnyxKeyboard::init(const QLocale & locale)
{
    keyboard_data_ = keyboard_data_factory_.getKeyboardData(locale);
}

void OnyxKeyboard::createLayout()
{
    handwriting_widget_.reset(new OnyxHandwritingWidget(this));

    createTop();
    createLeft();
    createMiddle();
    createRight();
    createBottom();
    createMenu();

    big_layout_.addWidget(&top_);
    layout_.addWidget(&left_);
    layout_.addWidget(&middle_);
    layout_.addWidget(&right_);

    big_layout_.setContentsMargins(1, 1, 1, 1);
    big_layout_.setSpacing(1);
    big_layout_.addLayout(&layout_);
    big_layout_.addWidget(&bottom_);

    big_layout_.addWidget(&menu_);

    // add handwriting widget
    big_layout_.addWidget(handwriting_widget_.get());
    handwriting_widget_->hide();

    connectWithChildren();
}

void OnyxKeyboard::connectWithChildren()
{
    connect(&top_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&left_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&middle_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&right_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&bottom_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&menu_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));

    connect(&top_, SIGNAL(outOfUp(CatalogView *, int, int)),
            this, SLOT(onOutOfUp(CatalogView *, int, int)));
    connect(&menu_, SIGNAL(outOfDown(CatalogView *, int, int)),
                this, SLOT(onOutOfDown(CatalogView *, int, int)));

    connect(&left_, SIGNAL(keyRelease(CatalogView *, QKeyEvent *)), this, SLOT(onViewKeyRelease(CatalogView *, QKeyEvent *)));
    connect(&middle_, SIGNAL(keyRelease(CatalogView *, QKeyEvent *)), this, SLOT(onViewKeyRelease(CatalogView *, QKeyEvent *)));
    connect(&right_, SIGNAL(keyRelease(CatalogView *, QKeyEvent *)), this, SLOT(onViewKeyRelease(CatalogView *, QKeyEvent *)));


    connect(handwriting_widget_.get(), SIGNAL(showKeyboard()),
            this, SLOT(onShowKeyboard()));
    connect(handwriting_widget_.get(),
            SIGNAL(handwritingKeyPressed(const QString &, const int &)),
            this,
            SLOT(onHandwritingKeyPressed(const QString &, const int &)));
}

void OnyxKeyboard::createTop()
{
    top_.data().clear();
    top_.setSubItemType(KeyboardKeyView::type());
    top_.setFixedHeight(keyboardKeyHeight());
    top_.setPreferItemSize(s_size);
    top_.setData(keyboard_data_->topCodes());
    top_.setFixedGrid(1, 10);
    top_.setMargin(CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN);
    top_.setSearchPolicy(CatalogView::NeighborFirst|CatalogView::AutoHorRecycle);
    top_.setNeighbor(&left_, CatalogView::DOWN);
    top_.setNeighbor(&middle_, CatalogView::DOWN);
    top_.setNeighbor(&right_, CatalogView::DOWN);
}

void OnyxKeyboard::createBottom()
{
    bottom_.data().clear();
    bottom_.setSubItemType(KeyboardKeyView::type());
    bottom_.setFixedHeight(keyboardKeyHeight());
    bottom_.setSearchPolicy(CatalogView::NeighborFirst|CatalogView::AutoHorRecycle);
    bottom_.setPreferItemSize(s_size);
    bottom_.setData(keyboard_data_->bottomCodes());
    bottom_.setFixedGrid(1, 10);
    bottom_.setMargin(CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN);
    bottom_.setNeighbor(&menu_, CatalogView::DOWN);
}

void OnyxKeyboard::createMenu()
{
    menu_.data().clear();
    menu_.setSubItemType(KeyboardKeyView::type());
    menu_.setFixedHeight(keyboardKeyHeight());
    menu_.setSearchPolicy(CatalogView::NeighborFirst|CatalogView::AutoHorRecycle);
    menu_.setPreferItemSize(s_size);

    ODatas menu_datas = keyboard_data_->menuCodes();
    menu_.setData(menu_datas);
    menu_.setFixedGrid(1, menu_datas.size());

    menu_.setMargin(CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN);
}

void OnyxKeyboard::createLeft()
{
    left_.data().clear();
    left_.setSubItemType(KeyboardKeyView::type());
    left_.setFixedHeight(keyboardKeyHeight()*3);
    left_.setPreferItemSize(s_size);
    left_.setData(keyboard_data_->leftCodes());
    left_.setFixedGrid(3, 3);
    left_.setMargin(CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN);
    left_.setNeighbor(&middle_, CatalogView::RIGHT);
    left_.setNeighbor(&right_, CatalogView::RECYCLE_RIGHT);
    left_.setNeighbor(&bottom_, CatalogView::DOWN);
}

void OnyxKeyboard::createMiddle()
{
    middle_.data().clear();
    middle_.setSubItemType(KeyboardKeyView::type());
    middle_.setFixedHeight(keyboardKeyHeight()*3);
    middle_.setPreferItemSize(s_size);
    middle_.setData(keyboard_data_->middleCodes());
    middle_.setFixedGrid(LETTER_ROWS, MIDDLE_COLS);
    middle_.setMargin(CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN);
    middle_.setNeighbor(&left_, CatalogView::LEFT);
    middle_.setNeighbor(&right_, CatalogView::RIGHT);
    middle_.setNeighbor(&bottom_, CatalogView::DOWN);
}

void OnyxKeyboard::createRight()
{
    right_.data().clear();
    right_.setSubItemType(KeyboardKeyView::type());
    right_.setFixedHeight(keyboardKeyHeight()*3);
    right_.setPreferItemSize(s_size);
    right_.setData(keyboard_data_->rightCodes());
    right_.setFixedGrid(LETTER_ROWS, RIGHT_COLS);
    right_.setMargin(CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN, CATALOG_MARGIN);
    right_.setNeighbor(&middle_, CatalogView::LEFT);
    right_.setNeighbor(&left_, CatalogView::RECYCLE_LEFT);
    right_.setNeighbor(&bottom_, CatalogView::DOWN);
}

void OnyxKeyboard::initFocus()
{
    if (middle_.visibleSubItems().size() > 0)
    {
        // set focus to central key of middle zone.
        middle_.setFocusTo(1, 1);
    }
}

bool OnyxKeyboard::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        onyx::screen::watcher().updateScreen();
    }
    return ret;
}

void OnyxKeyboard::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        menuItemActivated(item, user_data);
        return;
    }
    else
    {
        int key_code;
        QString key_text;
        if (item_data->contains(TAG_SPECIAL_KEY))
        {
            key_text = item_data->value(TAG_SPECIAL_KEY_TEXT).toString();
            key_code = item_data->value(TAG_SPECIAL_KEY).toInt();
        }
        else
        {
            key_text = item_data->value(TAG_TITLE).toString();
            key_code = key_text.at(0).unicode();
        }
        // Either use postEvent with new or sendEvent directly.
        QKeyEvent key_event(QEvent::KeyPress, key_code,  Qt::NoModifier, key_text);
        QApplication::sendEvent(parentWidget(), &key_event);
    }
}

void OnyxKeyboard::onOutOfUp(CatalogView *child, int row, int col)
{
    emit outOfUp(child, row, col);
}

void OnyxKeyboard::onOutOfDown(CatalogView *child, int row, int col)
{
    emit outOfDown(child, row , col);
}

void OnyxKeyboard::onViewKeyRelease(CatalogView *view, QKeyEvent *key)
{
    if (key->key() == Qt::Key_PageDown)
    {
        if (view == &left_)
        {
            middle_.setFocusTo(1, 1);
        }
        else if (view == &middle_)
        {
            right_.setFocusTo(1, 1);
        }
        else if (view == &right_)
        {
            left_.setFocusTo(1, 1);
        }
    }
    else if (key->key() == Qt::Key_PageUp)
    {
        if (view == &left_)
        {
            right_.setFocusTo(1, 1);
        }
        else if (view == &middle_)
        {
            left_.setFocusTo(1, 1);
        }
        else if (view == &right_)
        {
            middle_.setFocusTo(1, 1);
        }
    }
}

void OnyxKeyboard::menuItemActivated(ContentView *item, int user_data)
{
    int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
    if (KEYBOARD_MENU_SHIFT == menu_type)
    {
        shiftClicked();
        update();
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
    }
    else if(KEYBOARD_MENU_SYMBOL == menu_type)
    {
        symbolClicked();
        update();
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
    }
    else if(KEYBOARD_MENU_LANGUAGE == menu_type)
    {
        languageClicked();
    }
    else if(KEYBOARD_MENU_WRITE == menu_type)
    {
        writeFunctionClicked();
    }
}

void OnyxKeyboard::resetData(bool shift)
{
    top_.setData(keyboard_data_->topCodes(shift));
    left_.setData(keyboard_data_->leftCodes(shift));
    middle_.setData(keyboard_data_->middleCodes(shift));
    right_.setData(keyboard_data_->rightCodes(shift));
    bottom_.setData(keyboard_data_->bottomCodes(shift));
}

void OnyxKeyboard::shiftClicked()
{
    shift_ = !shift_;
    // reset the symbol menu item
    symbol_ = false;
    resetData(shift_);
}

void OnyxKeyboard::symbolClicked()
{
    symbol_ = !symbol_;
    if (!symbol_)
    {
        shift_ = true;
        shiftClicked();
    }
    else
    {
        top_.setData(keyboard_data_->topCodes(true, true));
        left_.setData(keyboard_data_->leftSymbolCodes());
        middle_.setData(keyboard_data_->middleSymbolCodes());
        right_.setData(keyboard_data_->rightSymbolCodes());
        bottom_.setData(keyboard_data_->bottomCodes(true));
    }
}

void OnyxKeyboard::languageClicked()
{
    OnyxKeyboardLanguageDialog dialog(language_, 0);
    onyx::screen::watcher().addWatcher(&dialog);
    int ret = dialog.exec();
    onyx::screen::watcher().removeWatcher(&dialog);
    onyx::screen::watcher().enqueue(0);

    if (ret != QDialog::Rejected)
    {
        QLocale selected = dialog.selectedLocale();
        if (selected.name() != language_.name())
        {
            setKeyboardLanguae(selected);
        }
    }
}

void OnyxKeyboard::setKeyboardLanguae(QLocale language)
{
    language_ = language;
    init(language);
    resetData(false);
    update();
}

void OnyxKeyboard::writeFunctionClicked()
{
    is_handwriting_ = true;

    int keyboard_width = this->width();
    int keyboard_height = this->height();

    top_.hide();
    left_.hide();
    middle_.hide();
    right_.hide();
    bottom_.hide();
    menu_.hide();

    handwriting_widget_->popup(keyboard_width, keyboard_height);

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void OnyxKeyboard::onShowKeyboard()
{
    is_handwriting_ = false;

    top_.show();
    left_.show();
    middle_.show();
    right_.show();
    bottom_.show();
    menu_.show();

    initFocus();

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

void OnyxKeyboard::onHandwritingKeyPressed(const QString &key_text,
        const int &key_code)
{
    QKeyEvent key_event(QEvent::KeyPress, key_code,  Qt::NoModifier, key_text);
    QApplication::sendEvent(parentWidget(), &key_event);
}

}   // namespace ui
