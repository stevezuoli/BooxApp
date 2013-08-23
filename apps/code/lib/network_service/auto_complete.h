#ifndef AUTO_COMPLETE_H
#define AUTO_COMPLETE_H

#include "ns_utils.h"

namespace network_service
{

class AutoComplete
{
    AutoComplete();
    AutoComplete(const AutoComplete & right);
public:
    ~AutoComplete();
    static AutoComplete * instance();

    void setFormHtml(QUrl url, QString data);
    bool existInForms(QUrl url);

    void setFormData(QUrl url, QString data);
    bool existInFormsData(QUrl url);

    bool evaluate(QUrl url);
    bool complete(QWebFrame * frame);

private:
    QString formUrl(QUrl url);

private:
    QMap<QString, QString>  forms_;
    QMap<QString, QString>  forms_data_;
};

};

#endif // AUTO_COMPLETE_H
