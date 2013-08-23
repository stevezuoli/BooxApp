#ifndef ADOBE_RENDER_POLICY_H
#define ADOBE_RENDER_POLICY_H

#include "adobe_utils.h"

using namespace vbf;
namespace adobe_view
{

class AdobeRenderPolicy : public RenderPolicy
{
public:
    AdobeRenderPolicy();
    virtual ~AdobeRenderPolicy();

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
