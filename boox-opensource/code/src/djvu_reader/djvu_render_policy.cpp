#include "djvu_render_policy.h"

using namespace vbf;
namespace djvu_reader
{

void addRequest( const int current_page,
                 const int offset,
                 const int total,
                 bool forward,
                 QVector<int> & result )
{
    int next_page = forward ? (current_page + offset) : (current_page - offset);
    if ((next_page >= 0 && next_page <= total))
    {
        result.push_back(next_page);
    }
}

DjVuRenderPolicy::DjVuRenderPolicy()
    : RenderPolicy()
{
}

DjVuRenderPolicy::~DjVuRenderPolicy()
{
}

void DjVuRenderPolicy::getRenderRequests(const int current_page,
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

    // update the requests by given sources
    updateRequests(result);
}

void DjVuRenderPolicy::nextPagesFirst( const int current_page,
                                       const int total,
                                       bool forward,
                                       QVector<int> & result )
{
    // current + 1
    addRequest(current_page, 1, total, forward, result);

    // current + 2
    addRequest(current_page, 2, total, forward, result);

    // current - 1
    addRequest(current_page, -1, total, forward, result);
}

void DjVuRenderPolicy::farawayPagesFirst( const int current_page,
                                          const int total,
                                          bool forward,
                                          QVector<int> & result )
{
    // current + 5
    addRequest(current_page, 5, total, forward, result);

    // current + 1
    addRequest(current_page, 1, total, forward, result);

    // current - 1
    addRequest(current_page, -1, total, forward, result);
}

void DjVuRenderPolicy::nearPagesFirst( const int current_page,
                                       const int total,
                                       QVector<int> & result )
{
    // current + 1
    addRequest(current_page, 1, total, true, result);

    // current - 1
    addRequest(current_page, -1, total, true, result);
}

}
