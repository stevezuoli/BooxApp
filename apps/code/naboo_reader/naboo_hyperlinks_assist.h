#ifndef NABOO_HYPERLINKS_ASSIST_H_
#define NABOO_HYPERLINKS_ASSIST_H_

#include "naboo_utils.h"

using namespace ui;
using namespace adobe_view;

namespace naboo_reader
{

class NabooView;
class NabooHyperlinksAssist : public QObject
{
    Q_OBJECT
public:
    NabooHyperlinksAssist( NabooView * view );
    ~NabooHyperlinksAssist();

    bool hitTest( const QPoint & mouse_pos,
                  Range & range,
                  AdobeLocationPtr & target );
    QList<Range> getHyperlinksInScreen();

    Range getCurrentHyperlink(AdobeLocationPtr & target);
    bool nextHyperlink();
    bool prevHyperlink();

private Q_SLOTS:
    void onCurrentPageChanged(const int current, const int total);

private:
    NabooView *view_;
    int       current_link_idx_;
};

};
#endif
