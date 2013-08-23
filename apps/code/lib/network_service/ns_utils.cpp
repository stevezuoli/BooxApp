#include "onyx/sys/sys.h"
#include "ns_utils.h"

namespace network_service
{

static const QString CACHE_DIR = ".network_service_cache";
static const QString DOWNLOAD_FILE_DIR = "downloads";

QString getCacheLocation()
{
#ifdef Q_WS_QWS
    QString path = ("/media/flash");
#else
    QString path = QDir::home().path();
#endif
    QDir dir(path);
    QString ret;
    if (!dir.exists(CACHE_DIR))
    {
        if (!dir.mkdir(CACHE_DIR))
        {
            return ret;
        }
    }

    if (dir.cd(CACHE_DIR))
    {
        ret = dir.absolutePath();
    }
    return ret;
}

QString getDownloadFileLocation()
{
    sys::SystemConfig conf;
    return conf.downloadFolder();
}

QUrl guessUrlFromString(const QString &string)
{
    QString url_str = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool has_schema = test.exactMatch(url_str);
    if (has_schema)
    {
        QUrl url = QUrl::fromEncoded(url_str.toUtf8(), QUrl::TolerantMode);
        if (url.isValid())
        {
            return url;
        }
    }

    // Might be a file.
    if (QFile::exists(url_str))
    {
        QFileInfo info(url_str);
        return QUrl::fromLocalFile(info.absoluteFilePath());
    }

    // Might be a shorturl - try to detect the schema.
    if (!has_schema)
    {
        int dot_index = url_str.indexOf(QLatin1Char('.'));
        if (dot_index != -1)
        {
            QString prefix = url_str.left(dot_index).toLower();
            QByteArray schema = (prefix == QLatin1String("ftp")) ? prefix.toLatin1() : "http";
            QUrl url = QUrl::fromEncoded(schema + "://" + url_str.toUtf8(), QUrl::TolerantMode);
            if (url.isValid())
            {
                return url;
            }
        }
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl url = QUrl::fromEncoded(string.toUtf8(), QUrl::TolerantMode);

    // finally for cases where the user just types in a hostname add http
    if (url.scheme().isEmpty())
    {
        url = QUrl::fromEncoded("http://" + string.toUtf8(), QUrl::TolerantMode);
    }
    return url;
}

}
