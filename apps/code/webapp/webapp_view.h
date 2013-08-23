#ifndef WEB_APP_VIEW_H_
#define WEB_APP_VIEW_H_

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>
#include "onyx/ui/ui.h"
#include "onyx/screen/screen_proxy.h"
#include "network_service/web_view.h"

using namespace ui;
using namespace network_service;

namespace webapp
{

class WebAppView : public WebView
{
    Q_OBJECT
public:
    WebAppView(QWidget *parent = 0,
               NetworkAccessManager * access_manager = 0,
               DownloadManager * download_manager = 0);
    ~WebAppView();
};

};   // namespace webapp

#endif
