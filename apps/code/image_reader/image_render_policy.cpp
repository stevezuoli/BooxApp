#include "image_render_policy.h"
#include "image_item.h"
#include "image_thumbnail.h"

using namespace vbf;
namespace image
{

// Compare the priority between two pages
int comparePriority(const ImageItem & p1,
                    const ImageItem & p2,
                    vbf::RenderPolicy * render_policy)
{
    int src_pri = render_policy->getPriority(p1.index());
    int dst_pri = render_policy->getPriority(p2.index());
    // the value of priority is lower, the priority is higher
    if (src_pri > dst_pri)
    {
        return -1;
    }
    else if (src_pri < dst_pri)
    {
        return 1;
    }
    return 0;
}

// -1 means p1 < p2
// 0 means p1 == p2
// 1 means p1 > p2
int compare(const ImageItem & p1,
            const ImageItem & p2,
            RenderPolicy * render_policy)
{
    // compare the existing of image
    if (p1.image() == 0 && p2.image() != 0)
    {
        return 1;
    }
    else if (p1.image() != 0 && p2.image() == 0)
    {
        return -1;
    }
    else if (p1.image() == 0 && p2.image() == 0)
    {
        return 0;
    }

    // compare lock
    if (p1.locked() && !p2.locked())
    {
        return 1;
    }
    else if (!p1.locked() && p2.locked())
    {
        return -1;
    }

    // Compare the priority in requests list
    return comparePriority(p1, p2, render_policy);
}

// Compare the priority between two thumbnails
int comparePriority(const ImageThumbnail & p1,
                    const ImageThumbnail & p2,
                    vbf::RenderPolicy * render_policy)
{
    if (p1.time() < p2.time())
    {
        return -1;
    }
    else if (p1.time() == p2.time())
    {
        return 0;
    }
    return 1;
}

int compare(const ImageThumbnail & p1,
            const ImageThumbnail & p2,
            vbf::RenderPolicy * render_policy)
{
    // compare the existing of image
    if (p1.image() == 0 && p2.image() != 0)
    {
        return 1;
    }
    else if (p1.image() != 0 && p2.image() == 0)
    {
        return -1;
    }
    else if (p1.image() == 0 && p2.image() == 0)
    {
        return 0;
    }

    // compare lock
    if (p1.locked() && !p2.locked())
    {
        return 1;
    }
    else if (!p1.locked() && p2.locked())
    {
        return -1;
    }

    // compare time
    return comparePriority(p1, p2, render_policy);
}

void addRequest(const int current_page,
                const int offset,
                const int total,
                bool forward,
                QVector<int> & result)
{
    int next_page = forward ? (current_page + offset) : (current_page - offset);
    if ((next_page > 0 && next_page <= total))
    {
        result.push_back(next_page);
    }
}

ImageRenderPolicy::ImageRenderPolicy()
    : RenderPolicy()
{
}

ImageRenderPolicy::~ImageRenderPolicy()
{
}

void ImageRenderPolicy::getRenderRequests(const int current_page,
                                          const int previous_page,
                                          const int total,
                                          QVector<int> & result)
{
    // the current page should always be set at the front of the queue
    result.clear();
    result.push_back(current_page);

    // get the step of previous page and current page
    int step = current_page - previous_page;

    // the current page should always be added
    switch (step)
    {
    case -1:
    case 1:
        {
            // prerender the next pages
            nextPagesFirst(current_page, total, step > 0, result);
        }
        break;
    case -5:
    case 5:
        {
            // prerender the next 5 pages
            farawayPagesFirst(current_page, total, step > 0, result);
        }
    case 0:
        {
            // prerender the nearby pages
            nearPagesFirst(current_page, total, result);
        }
        break;
    default:
        {
            // prerender the nearby pages
            nearPagesFirst(current_page, total, result);
        }
        break;
    }

    updateRequests(result);
}

void ImageRenderPolicy::nextPagesFirst(const int current_page,
                                       const int total,
                                       bool forward,
                                       QVector<int> & result)
{
    // current + 1
    addRequest(current_page, 1, total, forward, result);

    // current + 2
    addRequest(current_page, 2, total, forward, result);

    // current + 3
    //addRequest(current_page, 3, total, forward, result);

    // current + 4
    //addRequest(current_page, 4, total, forward, result);

    // current - 1
    //addRequest(current_page, -1, total, forward, result);
}

void ImageRenderPolicy::farawayPagesFirst(const int current_page,
                                          const int total,
                                          bool forward,
                                          QVector<int> & result)
{
    // current + 5
    addRequest(current_page, 5, total, forward, result);

    // current + 1
    addRequest(current_page, 1, total, forward, result);

    // current - 1
    addRequest(current_page, -1, total, forward, result);
}

void ImageRenderPolicy::nearPagesFirst(const int current_page,
                                       const int total,
                                       QVector<int> & result)
{
    // current + 1
    addRequest(current_page, 1, total, true, result);

    // current - 1
    addRequest(current_page, -1, total, true, result);
}

}
