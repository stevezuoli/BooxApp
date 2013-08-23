#include "password_model.h"

namespace network_service
{

PasswordModel::PasswordModel()
{
    data_.beginGroup("AutoComplete");
}

PasswordModel::~PasswordModel()
{
    data_.endGroup();
}

void PasswordModel::getModel(QStandardItemModel & passwords_model)
{
    QStringList hosts = data_.childGroups();
    passwords_model.setColumnCount(2);

    QStringList::iterator begin = hosts.begin();
    QStringList::iterator end   = hosts.end();
    int row = 0;
    for(QStringList::iterator iter  = begin; iter != end; ++iter, ++row)
    {
        QString host = (*iter);

        // title
        QStandardItem *url = new QStandardItem(QString::fromLocal8Bit(QByteArray::fromPercentEncoding(host.toLocal8Bit())));
        url->setData(host);
        url->setEditable(false);
        passwords_model.setItem(row, 0, url);

        // user name
        QString user_control = data_.value(host + "/form_username_control" ).toString();
        QString username = data_.value(host + "/" + user_control ).toString();
        QStandardItem * user = new QStandardItem(QString::fromLocal8Bit(QByteArray::fromPercentEncoding(username.toLocal8Bit())));
        user->setEditable(false);
        user->setTextAlignment(Qt::AlignCenter);
        passwords_model.setItem(row, 1, user);
    }

    passwords_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(QApplication::tr("URL")), Qt::DisplayRole);
    passwords_model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(QApplication::tr("User Name")), Qt::DisplayRole);
}

bool PasswordModel::removePassword(const QString & url)
{
    data_.remove(url);
    return true;
}

}
