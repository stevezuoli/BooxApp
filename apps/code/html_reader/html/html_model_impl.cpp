
#include "html_model_impl.h"

namespace html
{

const QString HtmlModelImpl::HTML_SCHEME = "html";

/// Implement html model.
HtmlModelImpl::HtmlModelImpl()
{
}

HtmlModelImpl::~HtmlModelImpl()
{
    close();
}

bool HtmlModelImpl::open(const QString &path)
{
    close();

    QFileInfo info(path);
    if (!info.exists())
    {
        return false;
    }

    // Update.
    path_ = path;
    current_location_.clear();
    current_location_.setScheme(HTML_SCHEME);
    current_location_.setPath(path);
    return true;
}

bool HtmlModelImpl::close()
{
    if (path_.isEmpty())
    {
        return false;
    }

    path_.clear();
    return true;
}

QString HtmlModelImpl::scheme()
{
    return HTML_SCHEME;
}

QUrl HtmlModelImpl::home()
{
    QUrl url;
    url.setScheme(HTML_SCHEME);
    url.setPath(path_);
    return url;
}

bool HtmlModelImpl::load(QUrl url, QByteArray & data)
{
    if (!resolve(url))
    {
        return false;
    }

    current_location_.setPath(url.path());
    QFile file(url.path());
    if (file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        return true;
    }
    return false;
}

bool HtmlModelImpl::resolve(QUrl & url)
{
    if (url.isRelative())
    {
        url = current_location_.resolved(url);
        return true;
    }
    return true;
}

bool HtmlModelImpl::supportTableOfContents()
{
    return false;
}

bool HtmlModelImpl::tableOfContents(QStandardItemModel &toc)
{
    return false;
}

bool HtmlModelImpl::urlFromIndex(QStandardItemModel &toc,
                                 const QModelIndex &selected,
                                 QUrl & url)
{
    return false;
}

bool HtmlModelImpl::indexFromUrl(QStandardItemModel &toc,
                                 const QUrl & url,
                                 QModelIndex &selected)
{
    return false;
}

bool HtmlModelImpl::nextPage(QUrl & url)
{
    return false;
}

bool HtmlModelImpl::prevPage(QUrl & url)
{
    return false;
}


}   // namespace html

