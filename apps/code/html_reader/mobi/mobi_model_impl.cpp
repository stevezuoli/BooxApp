#include <cassert>
#include <algorithm>
#include "mobi_model_impl.h"

namespace mobipocket
{

static const int a_size = 25;

static const QString MOBI_SCHEME = "mobi";

MobiModelImpl::MobiModelImpl()
{
}

MobiModelImpl::~MobiModelImpl()
{
}

bool MobiModelImpl::open(const QString &path)
{
    if (!stream_)
    {
        stream_.reset(new MobiStream(path.toStdString()));
    }

    if (!stream_->Open())
    {
        return false;
    }

    path_ = path;
    return true;
}

QString MobiModelImpl::scheme()
{
    return MOBI_SCHEME;
}

bool MobiModelImpl::close()
{
    path_.clear();
    stream_.reset(0);
    return true;
}

void MobiModelImpl::PreParse(char*& raw_html, size_t& size)
{
    char* p = raw_html;
    while (p)
    {
        char* q = strstr(p, "recindex");
        if (q == 0)
        {
            break;
        }

        // Replace "recindex" with "     src"
        if (!isalpha(*(q-1)))
        {
            strncpy(q, "     src", 8);
        }
        p = q + 8;
    }

    std::vector<int> anchors;

    p = raw_html;
    while (p)
    {
        char* q = strstr(p, "filepos=");
        if (!q)
        {
            break;
        }

        // Save pos.
        p = q + 8;
        int file_pos = 0;
        while (isdigit(*p))
        {
            file_pos = 10 * file_pos + (*p - '0');
            p++;
        }

        anchors.push_back(file_pos);

        // Replace "filepos=" with "href=#"
        strncpy(q, "href=\"#", 7);
        memmove(q+7, q+8, p-(q+8));
        p--;
        *p = '\"';
    }

    // Add <a name="xxxxxxxxxx"></a> inside anchors.
    std::sort(anchors.begin(), anchors.end());
    std::vector<int>::iterator it = anchors.begin();
    std::vector<int>::iterator it_tmp = it;
    for (; it != anchors.end(); )
    {
        if (it == anchors.begin())
        {
            ++it;
            continue;
        }

        if (*it == *it_tmp)
        {
            it = anchors.erase(it);
        }
        else
        {
            it_tmp = it++;
        }
    }

    size_t old_size = size;
    size += a_size * anchors.size();
    char* tmp = new char[size];

    p = raw_html;
    char* q = tmp;
    for (unsigned int i=0; i<anchors.size(); i++)
    {
        int file_pos = anchors[i];
        size_t bytes_copied = raw_html + file_pos - p;
        strncpy(q, p, bytes_copied);

        q += bytes_copied;
        p += bytes_copied;

        while (p != raw_html + old_size)
        {
            if (*p == '<')
            {
                if (*(p+1) == '/')
                {
                    *q++ = *p++;
                }
                else
                {
                    break;
                }
            }
            else if (*p == '>')
            {
                *q++ = *p++;
                if (*p != '<')
                {
                    break;
                }
            }
            else
            {
                *q++ = *p++;
            }
        }

        char buf[a_size + 1];
        sprintf(buf, "<a name=\"%010d\"></a>", file_pos);
        strncpy(q, buf, a_size);
        q += a_size;
    }

    if (p != raw_html + old_size)
    {
        strncpy(q, p, raw_html + old_size - p);
    }

    delete[] raw_html;
    raw_html = tmp;
}

/// Return the home url of mobi document.
QUrl MobiModelImpl::home()
{
    QUrl url;
    url.setScheme(MOBI_SCHEME);
    url.setPath("/home.html");
    return url;
}

bool MobiModelImpl::load(QUrl url, QByteArray & data)
{
    qDebug("path %s", qPrintable(url.path()));
    if (url.path() == "/home.html")
    {
        size_t size = stream_->GetSize();
        char *tmp = new char[size];
        stream_->Read(tmp, size);
        PreParse(tmp, size);
        data = data.fromRawData(tmp, size);
        return true;
    }

    QString imageIndex = url.path();
    if (imageIndex.startsWith("/"))
    {
        imageIndex.remove(0, 1);
    }

    int index = imageIndex.toInt() - 1;
    if (index >= 0 && index < stream_->images().size())
    {
        data = QByteArray::fromRawData(stream_->images().at(index).data,
                                       stream_->images().at(index).size);
        return true;
    }

    return true;
}

/// Generate model of table of content.
bool MobiModelImpl::supportTableOfContents()
{
    return false;
}

bool MobiModelImpl::tableOfContents(QStandardItemModel &toc)
{
    return false;
}

bool MobiModelImpl::urlFromIndex(QStandardItemModel &toc,
                                 const QModelIndex &selected,
                                 QUrl & url)
{
    return false;
}

bool MobiModelImpl::indexFromUrl(QStandardItemModel &toc,
                                 const QUrl & url,
                                 QModelIndex &selected)
{
    return false;
}

bool MobiModelImpl::nextPage(QUrl & url)
{
    return false;
}

bool MobiModelImpl::prevPage(QUrl & url)
{
    return false;
}

}
