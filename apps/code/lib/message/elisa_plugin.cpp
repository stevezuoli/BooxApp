
#include "elisa_plugin.h"
#include "elisa_content_parser.h"

static const QString HOST = "83.150.108.193";
static const QString USER_AGENT = "Mozilla/5.0 (QtEmbedded; N; Linux; fi-FI) AppleWebKit/527+ (KHTML, like Gecko, Safari/419.3) web_browser";

enum RequestId
{
    INVALID_ID = -1,
    CHECK_NEW_CONTENT_ID,
    CONFIRM_NEW_CONTENT_ID,
    UPDATE_ADOBE_ID,
};

ElisaPlugin::ElisaPlugin(const QString & host)
: host_(host)
{
    if (host_.isEmpty())
    {
        host_ = HOST;
    }
    connection_.setHost(host_);
    connect(&connection_, SIGNAL(requestFinished(int, bool)),
            this, SLOT(onRequestFinished(int, bool)));
}

ElisaPlugin::~ElisaPlugin()
{
}

bool ElisaPlugin::checkNewContent(bool check_all)
{
    QHttpRequestHeader header("POST", "/services/simplexml?method=contentservice.checknewcontent");
    header.setValue("Host", host());
    header.setValue("User-Agent", USER_AGENT);

    QByteArray post;
    if (check_all)
    {
        post += "&all=true";
    }
    else
    {
        post += "&all=false";
    }
    header.setContentLength(post.length());
    request_ids_.insert(CHECK_NEW_CONTENT_ID, connection_.request(header, post));
    return true;
}

bool ElisaPlugin::confirmNewContent(const QStringList ids)
{
    QHttpRequestHeader header("POST", "/services/simplexml?method=contentservice.confirmnewcontent");
    header.setValue("Host", host());
    header.setValue("User-Agent", USER_AGENT);

    QByteArray post;
    post.append("bookids=");
    for(int i = 0; i < ids.size() - 1; ++i)
    {
        post.append(ids.at(i));
        post.append("|");
    }
    post.append(ids.back());
    request_ids_.insert(CONFIRM_NEW_CONTENT_ID , connection_.request(header, post));
    return true;
}

/// Update Adobe id by invoking web service.
bool ElisaPlugin::updateAdobeId(QString & id)
{
    return true;
}

void ElisaPlugin::onRequestFinished( int id, bool error)
{
    if (error)
    {
        qDebug("Error occured in onRequestFinished id %d", id);
        return;
    }

    QByteArray data = connection_.readAll();
    if (request_ids_.value(CHECK_NEW_CONTENT_ID) == id)
    {
        content::Books books;
        ElisaContentParser::parseContents(books, data);
        emit newContentReady(books);
    }
    else if (request_ids_.value(CONFIRM_NEW_CONTENT_ID) == id)
    {
        ElisaContentParser::parseConfirmNewContents(data);
        emit newContentConfirmed();
    }
}

#ifndef QT_NO_OPENSSL
void ElisaPlugin::onSslErrors(const QList<QSslError> & errors)
{
    connection_.ignoreSslErrors();
}
#endif

