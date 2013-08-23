#ifndef PDF_RENDER_POLICY_H
#define PDF_RENDER_POLICY_H

#include "image_utils.h"

using namespace vbf;

namespace image
{

class ImageRenderPolicy : public vbf::RenderPolicy
{
public:
    ImageRenderPolicy();
    virtual ~ImageRenderPolicy();

    virtual void getRenderRequests(const int current_page,
                                   const int previous_page,
                                   const int total,
                                   QVector<int> & result);
private:
    void nextPagesFirst(const int current_page,
                        const int total,
                        bool forward,
                        QVector<int> & result);

    void farawayPagesFirst(const int current_page,
                           const int total,
                           bool forward,
                           QVector<int> & result);

    void nearPagesFirst(const int current_page,
                        const int total,
                        QVector<int> & result);
};

};

#endif
