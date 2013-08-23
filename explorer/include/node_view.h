#ifndef NODE_VIEW_H_
#define NODE_VIEW_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "node.h"
#include "file_node.h"

using namespace explorer::model;
using namespace ui;

namespace explorer
{

namespace model
{
    class Node;
}

namespace view
{

/// Present a single node for end user.
/// The node view can work in the following modes:
/// - List mode: Show small icon and name.
/// - Detailed mode: Show large icon, name, desc and title.
/// - Thumbnail mode: Show large icon and name.
/// The node view is used and organized by model view.
class NodeView : public QWidget
{
    Q_OBJECT

public:
    NodeView(QWidget * parent);
    ~NodeView(void);

public:
    bool setViewType(const ViewType mode);
    inline ViewType viewType() const { return view_mode_; }

    void setNode(Node *node);
    inline Node * node() { return node_; }

    void fieldResized(Field field, int x, int width);

    bool needPaint() const { return need_paint_;}

public Q_SLOTS:
    void select(bool select = true);
    bool updateMetadata();

Q_SIGNALS:
    void nodePressed(Node *node, const QPoint & press);
    void nodeReleased(Node *node, const QPoint & release);
    void nodeClicked(Node *node);

private:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    void paintListView(QPainter & painter);
    void paintDetailsView(QPainter & painter);
    void paintThumbnailView(QPainter & painter);

    void paintRatings(QPainter &painter);
    void updateRating(int rating);

    inline bool isLayoutDirty() const { return layout_dirty_; }
    void updateLayouts();
    void updateListViewLayout(QRect rect);
    void updateDetailsViewLayout(QRect rect);
    void updateThumbnailViewLayout(QRect rect);

    void sizeString(QString & string);

    void checkExpirationInfo(FileNode * node, QString & string);

    void updateScreen();
    bool checkImageThumbnail();

private:
    Node *node_;
    ViewType view_mode_;
    bool selected_;
    bool layout_dirty_;                    ///< Need to recalculate the layout.
    bool need_paint_;
    bool has_record_;
    QTextLayout name_layout_;
    QTextLayout type_layout_;
    QTextLayout size_layout_;
    QTextLayout last_access_layout_;

    // For details view.
    QTextLayout read_time_layout_;
    QTextLayout read_count_layout_;
    QTextLayout progress_layout_;

    QImage image_;                  ///< Used as icon.
    QPoint icon_pos_;

    static QImage star_black_image_;
    static QImage star_white_image_;
    QRect star_rect_;

    NO_COPY_AND_ASSIGN(NodeView);
};

}  /// end of view

}  /// end of explorer
#endif
