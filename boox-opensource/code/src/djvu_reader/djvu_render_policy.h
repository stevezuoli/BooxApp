#ifndef DJVU_RENDER_POLICY_H
#define DJVU_RENDER_POLICY_H

#include "djvu_utils.h"

using namespace vbf;
namespace djvu_reader
{

class DjVuRenderPolicy : public RenderPolicy
{
public:
    DjVuRenderPolicy();
    virtual ~DjVuRenderPolicy();

    virtual void getRenderRequests( const int current_page,
                                    const int previous_page,
                                    const int total,
                                    QVector<int> & result );
private:
    void nextPagesFirst( const int current_page,
                         const int total,
                         bool forward,
                         QVector<int> & result );

    void farawayPagesFirst( const int current_page,
                            const int total,
                            bool forward,
                            QVector<int> & result );

    void nearPagesFirst( const int current_page,
                         const int total,
                         QVector<int> & result );
};

};

#endif
