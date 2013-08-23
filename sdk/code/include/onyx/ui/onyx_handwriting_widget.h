#ifndef ONYX_HANDWRITING_WIDGET_H_
#define ONYX_HANDWRITING_WIDGET_H_


#include "onyx/base/base.h"
#include "ui_global.h"
#include "ui_utils.h"
#include "onyx/data/sketch_point.h"
#include "catalog_view.h"
#include "single_shot_timer.h"
#include "keyboard_utils.h"

namespace sketch
{
class SketchProxy;
};

using namespace ui;
using namespace sketch;

namespace handwriting
{

class HandwritingWidget;

class OnyxHandwritingWidget: public QWidget
{
    Q_OBJECT

public:
    explicit OnyxHandwritingWidget(QWidget *parent);
    ~OnyxHandwritingWidget();

    void popup(int width, int height);

Q_SIGNALS:
    void showKeyboard();
    void handwritingKeyPressed(const QString &key_text, const int &key_code);

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item,
            int user_data);

protected:
    void displayAssociatedChars(const QString & current_text);
    bool adjustAssociatedChar(const QString & dst_text, int index);

    void showEvent(QShowEvent * event);
    void hideEvent(QHideEvent * event);

private Q_SLOTS:
    void onFinishCharacterTimeOut();
    void onAutoSelect();
    void onStrokeStarted();
    void onPointAdded(SketchPoint point);
    void onStrokeAdded(const Points & points);

    void onTextSelected(const QString & text, int index);

private:
    void createLayout();
    void createMenu();
    void createCandidateCharList();
    void createCharSubsetList();
    void connectWithChildren();

    void initHandwrting();

    void charSubsetClicked(int row);
    void menuClicked(int menu_type);
    void keyClicked(OData *data);

    void setCandidateCharListData(const QStringList &char_list);
    void setCharSubsetListData();

private:
    QVBoxLayout big_layout_;
    QHBoxLayout sketch_widget_layout_;

    CatalogView menu_;
    CatalogView candidate_char_list_;
    scoped_ptr<HandwritingWidget> sketch_widget_;
    CatalogView char_subset_list_;

    ODatas menu_datas_;
    ODatas candidate_char_list_datas_;
    ODatas char_subset_list_datas_;

    QStandardItemModel char_subset_model_;
    scoped_ptr<sketch::SketchProxy> sketch_proxy_;

    OnyxSingleShotTimer finish_character_timer_;
    OnyxSingleShotTimer auto_select_timer_;
    QStringList candidates_;
    QString     current_text_;
};

}   // namespace handwriting

#endif
