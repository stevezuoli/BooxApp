#include "webapp_view.h"

using namespace network_service;

namespace webapp
{

WebAppView::WebAppView(QWidget *parent,
                       NetworkAccessManager * access_manager,
                       DownloadManager * download_manager)
: WebView(parent, access_manager, download_manager)
{
}

WebAppView::~WebAppView()
{
}

}
