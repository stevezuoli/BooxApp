#ifndef EXPLORER_SEARCH_WIDGET_H_
#define EXPLORER_SEARCH_WIDGET_H_

#include "onyx/ui/ui.h"
#include "branch_node.h"

using namespace ui;
using namespace explorer::model;

namespace explorer
{

namespace view
{

/// Define search context. Application may inherit this class
/// to implement its own search context.
class SearchContext
{
public:
    SearchContext();
    ~SearchContext();

public:
    void reset();

    const QString & pattern() const { return pattern_; }
    void setPattern(const QString &pattern);

    void includeSubDirs(bool include) { include_ = include; }
    bool includeSubDirs() { return include_; }

    void setNode(BranchNode *n) { node_ = n; }
    BranchNode * node() { return node_; }

    bool isStopped() { return stop_; }
    bool & mutable_stop() { return stop_; }
    void stop(bool s = true) { stop_ = s; }

private:
    QString pattern_;
    bool include_;
    bool stop_;
    BranchNode * node_;
};


/// Search dialog to enable user to search the documents.
class SearchDialog : public OnyxDialog
{
    Q_OBJECT

public:
    SearchDialog(QWidget *parent, SearchContext & ctx);
    ~SearchDialog(void);

public:
    int popup(int bottom_margin);

protected:
    void mouseMoveEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent * ke);
    bool event(QEvent * event);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void onSearchClicked();
    void onCloseClicked();
    void onTextChanged(const QString&);
    void forceUpdate();
    void onTimeout();

private:
    void createLayout();
    void updateChildrenWidgets(bool searching);
    void readyToSearch(bool forward);
    void updateTitle(const QString &message = QString());

private:
    QHBoxLayout  hbox_;

    OnyxLineEdit text_edit_;    ///< Input edit.
    OnyxPushButton   search_button_;   ///< Start to search.
    OnyxPushButton   clear_button_;   ///< Clear the text.
    OnyxCheckBox  sub_dir_;            ///< Include sub directory.

    KeyBoard      keyboard_;     ///< Keyboard.
    QTimer  searching_timer_;

    SearchContext & ctx_;
    bool searching_;
    bool update_parent_;
    bool update_whole_widget_;
    UpdateState update_state_;
};

}   // namespace view

}   // namespace explorer


#endif  // EXPLORER_SEARCH_WIDGET_H_
