#ifndef NETWORK_SERVICE_UTILS
#define NETWORK_SERVICE_UTILS

#include "onyx/ui/ui.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <QUrl>
#include <QMap>
#include <QDomElement>
#include <QSettings>
#include <QWebFrame>
#include <QWebSettings>
#include <QtGui/QDesktopServices>
#include <QtCore/QSettings>

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>

#include "onyx/wireless/wifi_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/dictionary/dictionary_manager.h"
#include "onyx/dictionary/dict_widget.h"
#include "onyx/sound/sound.h"
#include "onyx/tts/tts_widget.h"

namespace network_service
{

QString getCacheLocation();
QString getDownloadFileLocation();
QUrl guessUrlFromString(const QString &string);

};

#endif
